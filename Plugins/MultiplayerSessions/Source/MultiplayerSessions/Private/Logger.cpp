// (c) 2023 Will Roberts

#include "Logger.h"

#include "Engine/Engine.h"
#include "Logging/LogCategory.h"
#include "Logging/LogVerbosity.h"

DEFINE_LOG_CATEGORY(LogMultiplayerSessions);

void Logger::Log(FString Message)
{
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan, Message);
    UE_LOG(LogMultiplayerSessions, Display, TEXT("%s"), *Message);
}

void Logger::Error(FString Message)
{
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, Message);
    UE_LOG(LogMultiplayerSessions, Error, TEXT("%s"), *Message);
}