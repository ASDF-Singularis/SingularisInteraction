/* ====================================================================== *
 * SingularisInteractionComponent.cpp                                     *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2025 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2025 TrifingZW. All Rights Reserved.                     *
 * Created: 2025/11/04 | Author: TrifingZW                                *
 * Licensed under MIT License                                             *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   *
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   *
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      *
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 *
 * ====================================================================== */

#include "Components/SingularisInteractionComponent.h"

#include <Components/PrimitiveComponent.h>
#include <Engine/World.h>
#include <GameFramework/Controller.h>
#include <GameFramework/Pawn.h>
#include <UObject/UObjectGlobals.h>

#include "SingularisInteraction.h"
#include "Objects/SingularisInteractionBehavior.h"
#include "Objects/SingularisInteractionResponse.h"
#include "Objects/SingularisInteractionSession.h"
#include "Subsystems/SingularisInteractionSubsystem.h"
#include "Types/SingularisInteractionBehaviorType.h"
#include "Types/SingularisInteractionGameplayTags.h"

USingularisInteractionComponent::USingularisInteractionComponent()
{
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;

	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = true;

	bAutoActivate = true;
}

#if WITH_EDITOR
EDataValidationResult USingularisInteractionComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// 1) 逐管线校验标签有效性、标签唯一性与会话原型齐备
	TSet<FGameplayTag> DeclaredPipelineTags;
	for (const FSingularisInteractionPipeline& Pipeline : InteractionPipelines)
	{
		if (!Pipeline.PipelineTag.IsValid())
		{
			Context.AddError(FText::FromString(TEXT("交互管线集存在未配置的管线标签。")));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		if (DeclaredPipelineTags.Contains(Pipeline.PipelineTag))
		{
			Context.AddError(
				FText::FromString(
					FString::Printf(
						TEXT("管线标签 %s 重复；层级匹配只命中数组序中的首个管线，重复项永不生效。"),
						*Pipeline.PipelineTag.ToString()
					)
				)
			);
			Result = EDataValidationResult::Invalid;
		}
		DeclaredPipelineTags.Add(Pipeline.PipelineTag);

		if (!IsValid(Pipeline.Session))
		{
			Context.AddError(
				FText::FromString(
					FString::Printf(
						TEXT("管线标签 %s 未配置会话原型，命中后无法建立会话。"),
						*Pipeline.PipelineTag.ToString()
					)
				)
			);
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif

void USingularisInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 1) 在映射子系统登记交互目标组件
	RegisterInteractionMappings();

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] BeginPlay：交互组件初始化完成"),
		*GetNameSafe(GetOwner())
	);
}

void USingularisInteractionComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 1) 空表幂等返回
	if (ActiveSessions.IsEmpty())
		return;

	// 2) 第一遍：仅推进会话时钟与上下文快照（无外部调用，避免迭代中变更）
	TArray<TWeakObjectPtr<AActor>> PendingCancelInteractors;
	TArray<FSingularisInteractionSessionContext> PendingDriveContexts;

	for (auto& [Interactor, Record] : ActiveSessions)
	{
		if (!Interactor.IsValid() || !IsValid(Record.Session))
		{
			PendingCancelInteractors.Add(Interactor);
			continue;
		}

		Record.Elapsed += DeltaTime;
		PendingDriveContexts.Add(BuildSessionContext(Interactor, Record.PipelineTag, Record.Elapsed));
	}

	// 3) 第二遍：驱动会话推进（会话回调可能反向变更会话表，迭代的是上下文副本）
	for (const FSingularisInteractionSessionContext& Context : PendingDriveContexts)
	{
		const TWeakObjectPtr<AActor> InteractorKey(Context.Interactor.Get());
		if (!ActiveSessions.Contains(InteractorKey))
			continue;

		USingularisInteractionSession* Session = ActiveSessions[InteractorKey].Session;
		if (IsValid(Session))
			Session->Tick(Context, DeltaTime);
	}

	// 4) 统一结算失效会话（幂等）
	for (const TWeakObjectPtr<AActor>& Interactor : PendingCancelInteractors)
		SettleSession(Interactor, false);
}

void USingularisInteractionComponent::HostEmitSessionEvent_Implementation(
	USingularisInteractionSession* Session,
	const FGameplayTag& EventTag,
	FSingularisInteractionValue Value,
	const FInstancedStruct& Payload
)
{
	// 1) 定位会话归属会话（迟到发射静默吞并）
	const TWeakObjectPtr<AActor> Interactor = FindSessionInteractor(Session);
	if (!Interactor.IsValid())
		return;

	// 2) 以记录快照驱动（广播与响应可能反向变更会话表）
	const FSingularisInteractionSessionRecord Record = ActiveSessions[Interactor];
	DriveSessionEvent(Record, EventTag, Value, Payload);
}

void USingularisInteractionComponent::HostCompleteSession_Implementation(USingularisInteractionSession* Session)
{
	// 1) 定位会话归属会话并结算（幂等）
	SettleSession(FindSessionInteractor(Session), true);
}

void USingularisInteractionComponent::HostCancelSession_Implementation(USingularisInteractionSession* Session)
{
	// 1) 定位会话归属会话并结算（幂等）
	SettleSession(FindSessionInteractor(Session), false);
}

void USingularisInteractionComponent::SetEnabled(const bool IsEnabled)
{
	// 1) 幂等检查，状态未变更时直接返回
	if (bIsEnabled == IsEnabled)
		return;
	bIsEnabled = IsEnabled;

	// 2) 禁用时取消全部活跃会话（副本结算，避免迭代中变更）
	if (!IsEnabled)
	{
		TArray<TWeakObjectPtr<AActor>> Interactors;
		ActiveSessions.GetKeys(Interactors);
		for (const TWeakObjectPtr<AActor>& Interactor : Interactors)
			SettleSession(Interactor, false);
	}

	// 3) 广播状态变更事件
	if (IsEnabled)
		OnInteractionEnableEvent.Broadcast();
	else
		OnInteractionDisableEvent.Broadcast();

	// 4) 驱动行为执行副作用
	FSingularisInteractionBehaviorContext Context;
	Context.InteractionActor = GetOwner();
	Context.InteractionComponent = this;

	for (const auto& [Name, Description, Behavior] : InteractionBehaviors)
	{
		if (!IsValid(Behavior))
			continue;

		if (IsEnabled)
			Behavior->Enabled(Context);
		else
			Behavior->Disabled(Context);
	}

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] SetEnabled：交互状态 → %s"),
		*GetNameSafe(GetOwner()),
		IsEnabled ? TEXT("启用") : TEXT("禁用")
	);
}

void USingularisInteractionComponent::SetHovered(const bool IsHovered)
{
	// 1) 幂等检查，状态未变更时直接返回
	if (bIsHovered == IsHovered)
		return;
	bIsHovered = IsHovered;

	// 2) 广播状态变更事件
	if (IsHovered)
		OnInteractionHoverEvent.Broadcast();
	else
		OnInteractionUnhoverEvent.Broadcast();

	// 3) 驱动行为执行副作用
	FSingularisInteractionBehaviorContext Context;
	Context.InteractionActor = GetOwner();
	Context.InteractionComponent = this;

	for (const auto& [Name, Description, Behavior] : InteractionBehaviors)
	{
		if (!IsValid(Behavior))
			continue;

		if (IsHovered)
			Behavior->Hovered(Context);
		else
			Behavior->Unhovered(Context);
	}

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] SetHovered：悬浮状态 → %s"),
		*GetNameSafe(GetOwner()),
		IsHovered ? TEXT("悬浮") : TEXT("未悬浮")
	);
}

USingularisInteractionSession* USingularisInteractionComponent::GetInteractionSession(
	const FGameplayTag& PipelineTag
) const
{
	// 1) 请求标签无效时无匹配
	if (!PipelineTag.IsValid())
		return nullptr;

	// 2) 按数组序取首个标签层级命中的管线，返回其会话原型
	for (const FSingularisInteractionPipeline& Pipeline : InteractionPipelines)
	{
		if (Pipeline.PipelineTag.MatchesTag(PipelineTag) && IsValid(Pipeline.Session))
			return Pipeline.Session;
	}

	return nullptr;
}

void USingularisInteractionComponent::ReceiveInteractionIntent(
	AActor* Interactor,
	const FGameplayTag& PipelineTag,
	const FSingularisInteractionIntent& Intent
)
{
	// 1) 零信任校验（宿主 + 交互者 + 意图）
	if (!GetOwner())
		return;

	if (!IsValid(Interactor))
	{
		UE_LOG(
			LogSingularisInteraction,
			Warning,
			TEXT("[%s] ReceiveInteractionIntent：交互者无效"),
			*GetNameSafe(GetOwner())
		);
		return;
	}

	if (!Intent.IntentTag.IsValid() || !PipelineTag.IsValid())
		return;

	// 2) 禁用时拒绝新会话（广播取消反馈）
	if (!bIsEnabled)
	{
		if (Intent.IntentTag == SingularisInteraction_Intent_Press)
			RejectSession(Interactor, PipelineTag);
		return;
	}

	const TWeakObjectPtr<AActor> InteractorKey(Interactor);

	// 3) 建立路径：主张意图且无既有会话
	if (Intent.IntentTag == SingularisInteraction_Intent_Press && !ActiveSessions.Contains(InteractorKey))
	{
		BeginSession(Interactor, PipelineTag);
		return;
	}

	// 4) 转发路径：既有会话且标签一致，委托会话摄入
	if (!ActiveSessions.Contains(InteractorKey))
		return;

	const FSingularisInteractionSessionRecord& Record = ActiveSessions[InteractorKey];
	if (Record.PipelineTag != PipelineTag)
		return;

	if (!IsValid(Record.Session))
	{
		SettleSession(InteractorKey, false);
		return;
	}

	Record.Session->ReceiveIntent(BuildSessionContext(Record.Interactor, Record.PipelineTag, Record.Elapsed), Intent);
}

void USingularisInteractionComponent::RegisterInteractionMappings()
{
	// 1) 基础检查
	const UWorld* World = GetWorld();
	AActor* const OwnerActor = GetOwner();
	if (!IsValid(World) || !IsValid(OwnerActor))
	{
		UE_LOG(
			LogSingularisInteraction,
			Warning,
			TEXT("[%s] RegisterInteractionMappings：World 或宿主无效"),
			*GetNameSafe(OwnerActor)
		);
		return;
	}

	USingularisInteractionSubsystem* MappingSubsystem = World->GetSubsystem<USingularisInteractionSubsystem>();
	if (!IsValid(MappingSubsystem))
	{
		UE_LOG(
			LogSingularisInteraction,
			Warning,
			TEXT("[%s] RegisterInteractionMappings：交互映射子系统无效"),
			*GetNameSafe(OwnerActor)
		);
		return;
	}

	// 2) 逐条解析并登记交互目标组件
	auto RegisteredCount = 0;
	for (const auto& Reference : TargetComponentReferences)
	{
		UPrimitiveComponent* TargetComponent = Cast<UPrimitiveComponent>(Reference.GetComponent(OwnerActor));
		if (!IsValid(TargetComponent))
		{
			UE_LOG(
				LogSingularisInteraction,
				Warning,
				TEXT("[%s] RegisterInteractionMappings：交互目标引用解析失败或非碰撞组件，已跳过"),
				*GetNameSafe(OwnerActor)
			);
			continue;
		}

		MappingSubsystem->RegisterMapping(TargetComponent, this);
		++RegisteredCount;
	}

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] RegisterInteractionMappings：登记碰撞组件 %d 个"),
		*GetNameSafe(OwnerActor),
		RegisteredCount
	);
}

void USingularisInteractionComponent::BeginSession(AActor* Interactor, const FGameplayTag& PipelineTag)
{
	const TWeakObjectPtr<AActor> InteractorKey(Interactor);

	// 1) 管线匹配：取首个命中管线的会话原型
	USingularisInteractionSession* SessionArchetype = GetInteractionSession(PipelineTag);
	const FSingularisInteractionSessionContext GateContext = BuildSessionContext(InteractorKey, PipelineTag, 0.0f);

	// 2) 无匹配会话或开始判据拒绝：广播取消反馈并返回
	if (!IsValid(SessionArchetype) || !SessionArchetype->CanBegin(GateContext))
	{
		RejectSession(Interactor, PipelineTag);
		return;
	}

	// 3) 派生会话专属会话实例（以原型副本，宿主为本组件）
	USingularisInteractionSession* SessionInstance = DuplicateObject<USingularisInteractionSession>(
		SessionArchetype,
		this
	);
	if (!IsValid(SessionInstance))
	{
		RejectSession(Interactor, PipelineTag);
		return;
	}
	SessionInstance->BindSessionHost(this);

	// 4) 建立会话记录并启用时钟
	FSingularisInteractionSessionRecord& Record = ActiveSessions.Add(InteractorKey);
	Record.Interactor = InteractorKey;
	Record.PipelineTag = PipelineTag;
	Record.Session = SessionInstance;
	Record.Elapsed = 0.0f;
	RefreshSessionTick();

	// 5) 快照会话记录（后续广播与响应驱动可能反向变更会话表）
	const FSingularisInteractionSessionRecord RecordSnapshot = Record;

	// 6) 广播与驱动 Begin 事件
	DriveSessionEvent(
		RecordSnapshot,
		SingularisInteraction_Event_Begin,
		FSingularisInteractionValue(),
		FInstancedStruct()
	);

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] BeginSession：交互者 %s 建立会话，标签 %s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(Interactor),
		*PipelineTag.ToString()
	);

	// 7) 驱动会话开始（瞬时会话在此完成，会话可能已被结算，不得再触碰记录）
	SessionInstance->Begin(GateContext);
}

void USingularisInteractionComponent::RejectSession(AActor* Interactor, const FGameplayTag& PipelineTag)
{
	// 1) 组装拒绝反馈事件（会话未建立，不驱动响应）
	FSingularisInteractionSessionEvent Event;
	Event.EventTag = SingularisInteraction_Event_Cancel;
	Event.Context = BuildSessionContext(TWeakObjectPtr<AActor>(Interactor), PipelineTag, 0.0f);

	// 2) 本地广播取消反馈
	OnInteractionSessionEvent.Broadcast(Event);
}

void USingularisInteractionComponent::SettleSession(const TWeakObjectPtr<AActor>& Interactor, const bool bCompleted)
{
	// 1) 幂等校验：会话不存在时直接返回
	if (!ActiveSessions.Contains(Interactor))
		return;

	// 2) 摘除会话并快照记录
	const FSingularisInteractionSessionRecord Record = ActiveSessions[Interactor];
	ActiveSessions.Remove(Interactor);
	RefreshSessionTick();

	// 3) 广播与驱动结算事件（达成收敛为 1，中止归零）
	DriveSessionEvent(
		Record,
		bCompleted ? SingularisInteraction_Event_Complete : SingularisInteraction_Event_Cancel,
		FSingularisInteractionValue(bCompleted ? 1.0f : 0.0f),
		FInstancedStruct()
	);

	// 4) 驱动会话收尾
	if (IsValid(Record.Session))
		Record.Session->End(BuildSessionContext(Record.Interactor, Record.PipelineTag, Record.Elapsed));

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] SettleSession：交互者 %s 会话%s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(Record.Interactor.Get()),
		bCompleted ? TEXT("达成") : TEXT("中止")
	);
}

void USingularisInteractionComponent::DriveSessionEvent(
	const FSingularisInteractionSessionRecord& Record,
	const FGameplayTag& EventTag,
	const FSingularisInteractionValue& Value,
	const FInstancedStruct& Payload
)
{
	// 1) 组装事件（快照驱动，不回读会话表）
	FSingularisInteractionSessionEvent Event;
	Event.EventTag = EventTag;
	Event.Value = Value;
	Event.Payload = Payload;
	Event.Context = BuildSessionContext(Record.Interactor, Record.PipelineTag, Record.Elapsed);

	// 2) 本地广播
	OnInteractionSessionEvent.Broadcast(Event);

	// 3) 驱动匹配管线响应消费事件
	DriveSessionResponses(Event);

	// 4) 通知世界子系统（全局观察 / 遥测）
	if (const UWorld* World = GetWorld())
	{
		if (USingularisInteractionSubsystem* Subsystem = World->GetSubsystem<USingularisInteractionSubsystem>())
			Subsystem->NotifyWorldSessionEvent(this, Event);
	}
}

void USingularisInteractionComponent::DriveSessionResponses(const FSingularisInteractionSessionEvent& Event)
{
	// 1) 按数组顺序逐管线驱动命中标签的响应集
	for (const FSingularisInteractionPipeline& Pipeline : InteractionPipelines)
	{
		if (!Pipeline.PipelineTag.MatchesTag(Event.Context.PipelineTag))
			continue;

		for (const auto& [Name, Description, Response] : Pipeline.Responses)
		{
			if (IsValid(Response))
				Response->DispatchSessionEvent(Event);
		}
	}
}

FSingularisInteractionSessionContext USingularisInteractionComponent::BuildSessionContext(
	const TWeakObjectPtr<AActor>& Interactor,
	const FGameplayTag& PipelineTag,
	const float Elapsed
)
{
	// 1) 组装会话上下文（逻辑端不携带输入语义）
	FSingularisInteractionSessionContext Context;
	Context.Interactor = Interactor.Get();
	Context.Instigator = ResolveInstigator(Interactor.Get());
	Context.Avatar = GetOwner();
	Context.Target = GetOwner();
	Context.InteractionComponent = this;
	Context.PipelineTag = PipelineTag;
	Context.Elapsed = Elapsed;

	return Context;
}

void USingularisInteractionComponent::RefreshSessionTick() { SetComponentTickEnabled(!ActiveSessions.IsEmpty()); }

TWeakObjectPtr<AActor> USingularisInteractionComponent::FindSessionInteractor(
	const USingularisInteractionSession* Session
) const
{
	// 1) 线性反查会话归属会话（会话规模为个位数）
	for (const auto& [Interactor, Record] : ActiveSessions)
	{
		if (Record.Session == Session)
			return Interactor;
	}

	return nullptr;
}

APawn* USingularisInteractionComponent::ResolveInstigator(AActor* Interactor)
{
	// 1) Pawn 即触发者，控制器取其 Pawn
	if (const APawn* AsPawn = Cast<APawn>(Interactor))
		return const_cast<APawn*>(AsPawn);

	if (const AController* AsController = Cast<AController>(Interactor))
		return AsController->GetPawn();

	return nullptr;
}
