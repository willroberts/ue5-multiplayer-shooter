// (c) 2023 Will Roberts

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "TimerManager.h"

#include "Logger.h"

// UMultiplayerSessionsSubsystem constructs a new instance, binds delegates, and saves a pointer to the OnlineSubsystem's SessionInterface.
UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
    CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
    FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
    JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
    DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
    StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (!OnlineSubsystem)
    {
        Logger::Error(FString(TEXT("MultiplayerSessionsSubsystem: Failed to get OnlineSubsystem")));
        return;
    }

    IdentityInterface = OnlineSubsystem->GetIdentityInterface();
    SessionInterface = OnlineSubsystem->GetSessionInterface();
}

// CheckOnlineStatus returns true when the player is connected to Steam, EOS, etc.
bool UMultiplayerSessionsSubsystem::CheckOnlineStatus()
{
    if (!IdentityInterface)
    {
        Logger::Error(FString(TEXT("CheckOnlineStatus: Failed to get IdentityInterface")));
        return false;
    }

    if (!GetWorld())
    {
        Logger::Error(FString(TEXT("CheckOnlineStatus: Failed to get World")));
        return false;
    }

    const ULocalPlayer* Player = GetWorld()->GetFirstLocalPlayerFromController();
    if (!Player)
    {
        Logger::Error(FString(TEXT("CheckOnlineStatus: Failed to get Player")));
        return false;
    }

    ELoginStatus::Type Status = IdentityInterface->GetLoginStatus(Player->GetControllerId());
    if (Status == ELoginStatus::LoggedIn) return true;
    return false;
}

/* 
*  Bindable custom delegates
*/

// CreateSession destroys any existing session before creating a new online session.
void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
    if (!SessionInterface.IsValid())
    {
        Logger::Error(FString(TEXT("CreateSession: Failed to get SessionInterface")));
        return;
    }

    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession)
    {
        Logger::Log(FString(TEXT("CreateSession: Destroying existing session...")));
        bCreateSessionOnDestroy = true;
        LastNumPublicConnections = NumPublicConnections;
        LastMatchType = MatchType;

        DestroySession();
    }

    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

    LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
    LastSessionSettings->bAllowJoinInProgress = true;
    LastSessionSettings->bAllowJoinViaPresence = true;
    LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
    LastSessionSettings->bShouldAdvertise = true;
    LastSessionSettings->bUseLobbiesIfAvailable = true; // Needed for UE 5.0+.
    LastSessionSettings->bUsesPresence = true;
    LastSessionSettings->BuildUniqueId = 1; // Share sessions across builds.
    LastSessionSettings->NumPublicConnections = NumPublicConnections;
    LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        Logger::Error(FString(TEXT("CreateSession: Failed to get player's unique net ID")));
        return;
    }

    SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings);
}

// FindSessions searches for sessions and saves the results.
void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
    if (!SessionInterface.IsValid())
    {
        Logger::Error(FString(TEXT("FindSessions: Failed to get SessionInterface")));
        return;
    }

    // Configure search parameters.
    FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
    LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
    LastSessionSearch->MaxSearchResults = MaxSearchResults;
    LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
    LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
    LastSessionSearch->TimeoutInSeconds = 10.f;

    // Use first local player's unique net ID to find sessions.
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    Logger::Log(FString(TEXT("FindSessions: Sending search request")));
    SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef());
}

// JoinSession joins the specified game session with a player's unique ID.
void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult &SessionResult)
{
    if (!SessionInterface.IsValid())
    {
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        return;
    }
    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult);
}

// DestroySession destroys the current session.
void UMultiplayerSessionsSubsystem::DestroySession()
{
    if (!SessionInterface.IsValid())
    {
        MultiplayerOnDestroySessionComplete.Broadcast(false);
        return;
    }
    DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
    SessionInterface->DestroySession(NAME_GameSession);
}

// StartSession marks the online session as in-progress.
void UMultiplayerSessionsSubsystem::StartSession()
{
    if (!SessionInterface.IsValid())
    {
        MultiplayerOnStartSessionComplete.Broadcast(false);
        return;
    }
    StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);
    SessionInterface->StartSession(NAME_GameSession);
}

/*
* Delegate callbacks
*/

// OnCreateSessionComplete clears its delegate handle and broadcasts its result.
void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (!SessionInterface)
    {
        Logger::Error(FString(TEXT("OnCreateSessionComplete: Failed to get SessionInterface")));
        return;
    }
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

// OnFindSessionsComplete clears its delegate handle and broadcasts its result.
void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    Logger::Log(FString(TEXT("OnFindSessionsComplete: Handling response")));
    if (!SessionInterface)
    {
        Logger::Error(FString(TEXT("OnFindSessionsComplete: Failed to get SessionInterface")));
        return;
    }
    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

    if (LastSessionSearch->SearchResults.Num() <= 0)
    {
        Logger::Error(FString(TEXT("OnFindSessionsComplete: No results found")));
        MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
        return;
    }
    Logger::Log(FString(TEXT("OnFindSessionsComplete: Broadcasting results")));
    MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

// OnJoinSessionComplete clears its delegate handle and broadcasts its result.
void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (!SessionInterface)
    {
        Logger::Error(FString(TEXT("OnJoinSessionComplete: Failed to get SessionInterface")));
        return;
    }
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
    MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

// OnDestroySessionComplete clears its delegate handle and broadcasts its result.
// If `bCreateSessionOnDestroy` is true, this also creates a new online session.
void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (!SessionInterface)
    {
        Logger::Error(FString(TEXT("OnDestroySessionComplete: Failed to get SessionInterface")));
        return;
    }
    SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
    MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);

    if (bWasSuccessful && bCreateSessionOnDestroy)
    {
        Logger::Log(FString(TEXT("OnDestroySessionComplete: Automatically creating new session")));
        bCreateSessionOnDestroy = false;
        CreateSession(LastNumPublicConnections, LastMatchType);
    }
}

// OnStartSessionComplete clears its delegate handle and broadcasts its result.
void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (!SessionInterface)
    {
        Logger::Error(FString(TEXT("OnStartSessionComplete: Failed to get SessionInterface")));
        return;
    }
    SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
    MultiplayerOnStartSessionComplete.Broadcast(bWasSuccessful);
}