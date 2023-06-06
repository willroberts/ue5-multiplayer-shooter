// (c) 2023 Will Roberts

#include "DebugMenu.h"
#include "Logger.h"

#include "Components/Button.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

// AddMultiplayerDebugMenu implements the user-facing setup method for new debug menus.
// This function is callable from Blueprints.
void UDebugMenu::AddMultiplayerDebugMenu(
    int32 MaxSearchResults,
    int32 NumPlayers,
    FString GameMode,
    FString LobbyMap
) {
    // Save arguments for later reference.
    SessionSearchLimit = MaxSearchResults;
    NumPublicConnections = NumPlayers;
    MatchType = GameMode;
    LobbyMapPath = FString::Printf(TEXT("%s?listen"), *LobbyMap);

    // Add the widget to the viewport.
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    // Get the player controller from the World.
    UWorld* World = GetWorld();
    if (!World)
    {
        Logger::Error(FString(TEXT("Error: Failed to get World")));
        return;
    }
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        Logger::Error(FString(TEXT("Error: Failed to get PlayerController")));
        return;
    }

    // Configure mouse input for the menu widget.
    /*
    FInputModeUIOnly InputModeData;
    InputModeData.SetWidgetToFocus(TakeWidget());
    InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PlayerController->SetInputMode(InputModeData);
    PlayerController->SetShowMouseCursor(true);
    */

    // Get the Subsystem from the GameInstance.
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        Logger::Error(FString(TEXT("Error: Failed to get GameInstance")));
        return;
    }
    MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
    if (!MultiplayerSessionsSubsystem)
    {
        Logger::Error(FString(TEXT("Error: Failed to load Multiplayer plugin")));
        return;
    }

    // Bind callbacks.
    MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
    MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
    MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
    MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
    MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
}

/****************
Protected Methods
****************/

// Initialize adds click handlers to the Host and Join buttons.
bool UDebugMenu::Initialize()
{
    if (!Super::Initialize())
    {
        Logger::Error(FString(TEXT("Error: Failed to initialize DebugMenu")));
        return false;
    }

    if (!HostButton) Logger::Error(FString(TEXT("Error: HostButton not found")));
    else HostButton->OnClicked.AddDynamic(this, &UDebugMenu::HostButtonClicked);

    if (!JoinButton) Logger::Error(FString(TEXT("Error: Joinutton not found")));
    else JoinButton->OnClicked.AddDynamic(this, &UDebugMenu::JoinButtonClicked);

    return true;
}

// NativeDestruct calls the custom UDebugMenu::Destroy() function when the parent widget is destroyed.
void UDebugMenu::NativeDestruct()
{
    Destroy();
    Super::NativeDestruct();
}

// OnCreateSession is the delegate callback for session creation.
// When session creation was successful, initiates server travel to the lobby map.
void UDebugMenu::OnCreateSession(bool bWasSuccessful)
{
    //Logger::Log(FString(TEXT("Menu->OnCreateSession callback fired")));

    if (!bWasSuccessful)
    {
        Logger::Error(FString(TEXT("Error: Failed to create session")));
        HostButton->SetIsEnabled(true);
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        Logger::Error(FString(TEXT("Error: Failed to get World")));
        return;
    }

    //Logger::Log(FString::Printf(TEXT("Menu->OnCreateSession: Initiating server travel to map %s"), *LobbyMapPath));
    bWasSuccessful = World->ServerTravel(*LobbyMapPath);
    if (!bWasSuccessful)
    {
        Logger::Error(FString(TEXT("Error: Server travel failed")));
    }
}

// OnFindSessions is the delegate callback for session search.
// When a valid session is found, initiate a session join.
void UDebugMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
    //Logger::Log(FString(TEXT("Menu->OnFindSessions: Starting")));
    if (!bWasSuccessful || SessionResults.Num() == 0)
    {
        Logger::Error(FString(TEXT("Error: No sessions found")));
        JoinButton->SetIsEnabled(true);
        return;
    }

    if (!MultiplayerSessionsSubsystem)
    {
        Logger::Error(FString(TEXT("Error: Failed to load Multiplayer plugin")));
        return;
    }

    for (auto Result : SessionResults)
    {
        FString SettingsValue;
        Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
        if (SettingsValue != MatchType) continue;

        MultiplayerSessionsSubsystem->JoinSession(Result);
        return;
    }

    Logger::Error(FString(TEXT("Error: No sessions matched")));
}

// OnJoinSession is the delegate callback for session joins.
// When joining succeeds, initiate client travel to the session's platform-specific connection address.
void UDebugMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
    // Check the result of the call.
    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        JoinButton->SetIsEnabled(true);
        Logger::Error(FString(TEXT("Error: Failed to join session")));
        return;
    }

    // Get a pointer to the SessionInterface from the OnlineSubsystem.
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (!OnlineSubsystem)
    {
        Logger::Error(FString(TEXT("Error: Failed to get OnlineSubsystem")));
        return;
    }
    IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        Logger::Error(FString(TEXT("Error: Failed to get SessionInterface")));
        return;
    }

    // Get the platform-specific address of the session.
    FString Address;
    SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

    // Get the PlayerController and initiate client travel to the session.
    APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
    if (!PlayerController)
    {
        Logger::Error(FString(TEXT("Error: Failed to get PlayerController")));
        return;
    }

    //Logger::Log(FString(TEXT("Menu->OnJoinSession: Initiating client travel")));
    PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

// OnDestroySession is the delegate callback for session destruction.
void UDebugMenu::OnDestroySession(bool bWasSuccessful)
{
    if (!bWasSuccessful)
    {
        Logger::Error(FString(TEXT("Error: Failed to destroy session")));
        return;
    }
    //Logger::Log(FString(TEXT("Menu->OnDestroySession: Destroyed session")));
}

// OnStartSession is the delegate callback for session initiation.
void UDebugMenu::OnStartSession(bool bWasSuccessful)
{
    if (!bWasSuccessful)
    {
        Logger::Error(FString(TEXT("Error: Failed to start session")));
        return;
    }
    //Logger::Log(FString(TEXT("Menu->OnStartSession: Started session")));
}

/**************
Private Methods
**************/

// Destroy the menu widget and return control to the player controller.
void UDebugMenu::Destroy()
{
    // Remove the Widget from the UI.
    RemoveFromParent();
}

// HostButtonClicked temporarily disables the Host button before initiating session creation.
void UDebugMenu::HostButtonClicked()
{
    HostButton->SetIsEnabled(false);

    if (!MultiplayerSessionsSubsystem)
    {
        Logger::Error(FString(TEXT("Error: Failed to load Multiplayer plugin")));
        HostButton->SetIsEnabled(true);
        return;
    }

    // Check for valid online ID.
    if (!MultiplayerSessionsSubsystem->CheckOnlineStatus())
    {
        Logger::Error(FString(TEXT("Error: Not logged into Steam!")));
        HostButton->SetIsEnabled(true);
        return;
    }
    else
    {
        Logger::Log(FString::Printf(TEXT("Error: OnlineStatus: %d"), MultiplayerSessionsSubsystem->CheckOnlineStatus()));
    }

    // Create a session.
    /*Logger::Log(FString::Printf(
        TEXT("Menu->HostButtonClicked: Creating session with match type: %s, connections: %d"),
        *MatchType,
        NumPublicConnections
    ));*/
    MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType); 
}

// JoinButtonClicked temporarily disables the Join button before initiating session search.
void UDebugMenu::JoinButtonClicked()
{
    JoinButton->SetIsEnabled(false);

    if (!MultiplayerSessionsSubsystem)
    {
        Logger::Error(FString(TEXT("Error: Failed to load Multiplayer plugin")));
        JoinButton->SetIsEnabled(true);
        return;
    }

    // Check for valid online ID.
    if (!MultiplayerSessionsSubsystem->CheckOnlineStatus())
    {
        Logger::Error(FString(TEXT("Error: Not logged into Steam!")));
        JoinButton->SetIsEnabled(true);
        return;
    }

    // Search for sessions.
    MultiplayerSessionsSubsystem->FindSessions(SessionSearchLimit);
}