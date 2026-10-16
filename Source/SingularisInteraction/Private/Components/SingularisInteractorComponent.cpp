/* ====================================================================== *
 * SingularisInteractorComponent.cpp                                      *
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

#include "Components/SingularisInteractorComponent.h"

#include <Engine/World.h>
#include <GameFramework/Controller.h>
#include <GameFramework/Pawn.h>
#include <GameFramework/PlayerController.h>
#include <Math/UnrealMathUtility.h>
#include <UObject/UObjectGlobals.h>

#include "SingularisInteraction.h"
#include "Components/SingularisInteractionComponent.h"
#include "Drivers/SingularisInteractionEnhancedInputDriver.h"
#include "Subsystems/SingularisInteractionSubsystem.h"
#include "Types/SingularisInteractionGameplayTags.h"

USingularisInteractorComponent::USingularisInteractorComponent()
{
	SetIsReplicatedByDefault(false);

	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bCanEverTick = true;

	bAutoActivate = true;

	InteractionQueryer = CreateDefaultSubobject<USingularisInteractionQueryer>("InteractionQueryer");
	InteractionDriver = CreateDefaultSubobject<USingularisInteractionEnhancedInputDriver>("InteractionDriver");
}

void USingularisInteractorComponent::BeginPlay()
{
	Super::BeginPlay();

	// 1) 预解析宿主玩家控制器
	GetInteractorPlayerController();

	// 2) 装配驱动器（增强输入驱动器在非本地玩家交互者上自动空转）
	if (IsValid(InteractionDriver))
		InteractionDriver->InitializeDriver(this);

	// 3) 刷新输入装配至当前目标状态
	if (IsValid(InteractionDriver))
		InteractionDriver->OnTargetChanged(nullptr, CurrentQueryerResult.InteractionComponent);

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] BeginPlay：交互者组件初始化完成"),
		*GetNameSafe(GetOwner())
	);
}

void USingularisInteractorComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 1) 视线查询锁定目标（仅本地玩家交互者）
	Query();

	// 2) 推进本地镜像会话
	MirrorTick(DeltaTime);
}

void USingularisInteractorComponent::HostEmitSessionEvent_Implementation(
	USingularisInteractionSession* Session,
	const FGameplayTag& EventTag,
	FSingularisInteractionValue Value,
	const FInstancedStruct& Payload
)
{
	// 1) 仅接受当前镜像实例（冲刷后的迟到发射静默吞并）
	if (!bMirrorActive || Session != MirrorSession)
		return;

	// 2) 连续量与领域事件：本地会话流全量广播
	BroadcastInteractorSession(DriveTarget.Get(), DrivePipelineTag, EventTag, Value, Payload);
}

void USingularisInteractorComponent::HostCompleteSession_Implementation(
	USingularisInteractionSession* Session,
	const FInstancedStruct& Payload
)
{
	// 1) 镜像达成：结算
	if (bMirrorActive && Session == MirrorSession)
		EndMirror(true, Payload);
}

void USingularisInteractorComponent::HostCancelSession_Implementation(
	USingularisInteractionSession* Session,
	const FInstancedStruct& Payload
)
{
	// 1) 镜像中止：结算
	if (bMirrorActive && Session == MirrorSession)
		EndMirror(false, Payload);
}

void USingularisInteractorComponent::SetTarget(USingularisInteractionComponent* NewTarget)
{
	// 1) 组装最小查询结果（外部注入目标模式）
	FSingularisInteractionQueryerResult QueryerResult;
	if (IsValid(NewTarget))
	{
		QueryerResult.InteractionActor = NewTarget->GetOwner();
		QueryerResult.InteractionComponent = NewTarget;
	}

	// 2) 统一经查询结果通路应用（幂等比较 + 全量副作用）
	SetQueryerResult(QueryerResult);
}

bool USingularisInteractorComponent::SubmitInteractionIntent(
	USingularisInteractionComponent* Target,
	const FGameplayTag PipelineTag,
	const FGameplayTag IntentTag,
	const FSingularisInteractionValue& Value
)
{
	// 1) 零信任校验
	if (!IsValid(Target) || !PipelineTag.IsValid() || !IntentTag.IsValid())
		return false;

	AActor* InteractorActor = GetOwner();
	if (!IsValid(InteractorActor))
		return false;

	// 2) 单驱动卫语句：进行中的驱动仅接受同目标同标签意图
	if (bDriveActive && (DriveTarget.Get() != Target || DrivePipelineTag != PipelineTag))
		return false;

	// 3) 组装意图
	FSingularisInteractionIntent Intent;
	Intent.IntentTag = IntentTag;
	Intent.Value = Value;

	// 4) 本地玩家交互者：本地会话流
	if (IsLocalPlayerInteractor())
	{
		if (IntentTag == SingularisInteraction_Intent_Press)
		{
			bDriveActive = true;
			DriveTarget = Target;
			DrivePipelineTag = PipelineTag;

			USingularisInteractionSession* SessionArchetype = Target->GetInteractionSession(PipelineTag);

			// 4.1) 无会话原型：本地无会话流可产出，即刻冲刷驱动
			if (!IsValid(SessionArchetype))
			{
				UE_LOG(
					LogSingularisInteraction,
					Warning,
					TEXT("[%s] SubmitInteractionIntent：管线标签 %s 无会话原型，镜像未派生"),
					*GetNameSafe(InteractorActor),
					*PipelineTag.ToString()
				);
				FlushDrive();
			}
			else
			{
				// 4.2) 广播 Begin 并派生镜像会话
				BroadcastInteractorSession(
					Target,
					PipelineTag,
					SingularisInteraction_Event_Begin,
					FSingularisInteractionValue(),
					FInstancedStruct()
				);
				BeginMirror(SessionArchetype);
			}
		}
		else
			FeedMirrorIntent(Intent);
	}
	else if (IntentTag == SingularisInteraction_Intent_Press)
	{
		// 5) 其余交互者（AI / 脚本）：登记驱动状态供单驱动卫语句与结算清理
		bDriveActive = true;
		DriveTarget = Target;
		DrivePipelineTag = PipelineTag;
	}

	// 6) 经世界子系统单一入口提交
	if (UWorld* World = GetWorld())
	{
		if (USingularisInteractionSubsystem* Subsystem = World->GetSubsystem<USingularisInteractionSubsystem>())
			Subsystem->SubmitInteractionIntent(InteractorActor, Target, PipelineTag, Intent);
	}

	return true;
}

bool USingularisInteractorComponent::BeginInteraction(
	USingularisInteractionComponent* Target,
	const FGameplayTag PipelineTag
)
{
	return SubmitInteractionIntent(
		Target,
		PipelineTag,
		SingularisInteraction_Intent_Press,
		FSingularisInteractionValue()
	);
}

bool USingularisInteractorComponent::EndInteraction(
	USingularisInteractionComponent* Target,
	const FGameplayTag PipelineTag
)
{
	return SubmitInteractionIntent(
		Target,
		PipelineTag,
		SingularisInteraction_Intent_Release,
		FSingularisInteractionValue()
	);
}

APlayerController* USingularisInteractorComponent::GetInteractorPlayerController()
{
	// 1) 缓存有效时直接返回
	if (OwnerPlayerController.IsValid())
		return OwnerPlayerController.Get();

	// 2) 惰性解析：宿主控制器，或宿主 Pawn 的控制器
	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
		return nullptr;

	APlayerController* PlayerController = Cast<APlayerController>(OwnerActor);
	if (!IsValid(PlayerController))
	{
		if (const APawn* OwnerPawn = Cast<APawn>(OwnerActor))
			PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	}

	OwnerPlayerController = PlayerController;
	return PlayerController;
}

bool USingularisInteractorComponent::IsLocalPlayerInteractor()
{
	const APlayerController* PlayerController = GetInteractorPlayerController();
	return IsValid(PlayerController) && PlayerController->IsLocalController();
}

// ReSharper disable once CppPassValueParameterByConstReference
void USingularisInteractorComponent::HandleTargetSessionEvent(FSingularisInteractionSessionEvent Event)
{
	// 1) 仅关注本交互者会话：目标随绑定隐式确定（绑定与解绑均随目标迁移），零信任丢弃失效目标
	USingularisInteractionComponent* const Target = GetCurrentTarget();
	if (!IsValid(Target)
		|| (GetOwner() != Event.Context.Instigator.Get() && GetOwner() != Event.Context.Controller.Get()))
		return;

	// 2) 本地玩家：本地会话流自持，不消费会话端事件
	if (IsLocalPlayerInteractor())
		return;

	// 3) 其余交互者（AI / 脚本）：结算先冲刷驱动，其余事件直接转发
	if (Event.EventTag == SingularisInteraction_Event_Complete ||
		Event.EventTag == SingularisInteraction_Event_Cancel)
		FlushDrive();

	OnInteractorSessionEvent.Broadcast(Target, Event);
}

void USingularisInteractorComponent::Query()
{
	// 1) 仅本地玩家交互者且配置查询器时执行
	if (!IsLocalPlayerInteractor() || !IsValid(InteractionQueryer))
		return;

	APlayerController* PlayerController = OwnerPlayerController.Get();
	if (!IsValid(PlayerController))
		return;

	// 2) 获取本地玩家视线数据并构建查询参数
	FVector ViewLoc;
	FRotator ViewRot;
	PlayerController->GetPlayerViewPoint(ViewLoc, ViewRot);

	FSingularisInteractionQueryerParams QueryerParams;
	QueryerParams.ViewLoc = ViewLoc;
	QueryerParams.ViewRot = ViewRot;
	QueryerParams.IgnoredActor = PlayerController->GetPawn();

	// 3) 执行查询并写入结果
	FSingularisInteractionQueryerResult QueryerResult;
	InteractionQueryer->Query(QueryerResult, QueryerParams);
	SetQueryerResult(QueryerResult);
}

void USingularisInteractorComponent::SetQueryerResult(
	const FSingularisInteractionQueryerResult& QueryerResult
)
{
	// 1) 幂等性检查，若状态未变更则直接返回
	if (CurrentQueryerResult == QueryerResult)
		return;

	// 2) 捕获旧状态后写入新状态并应用副作用
	const FSingularisInteractionQueryerResult OldQueryerResult = CurrentQueryerResult;
	CurrentQueryerResult = QueryerResult;
	ApplyQueryerResult(OldQueryerResult);

	// 3) 广播交互目标变更
	OnInteractionTargetChangedEvent.Broadcast(
		OldQueryerResult.InteractionComponent,
		CurrentQueryerResult.InteractionComponent
	);

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] SetQueryerResult：交互目标 %s → %s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(OldQueryerResult.InteractionComponent),
		*GetNameSafe(CurrentQueryerResult.InteractionComponent)
	);
}

void USingularisInteractorComponent::ApplyQueryerResult(
	const FSingularisInteractionQueryerResult& OldQueryerResult
)
{
	USingularisInteractionComponent* OldTarget =
		OldQueryerResult.IsInteractionValid() ? OldQueryerResult.InteractionComponent : nullptr;
	USingularisInteractionComponent* NewTarget =
		CurrentQueryerResult.IsInteractionValid() ? CurrentQueryerResult.InteractionComponent : nullptr;

	// 1) 目标迁移：向旧目标提交撤回、冲刷驱动、解绑会话事件并取消悬浮
	if (IsValid(OldTarget))
	{
		if (bDriveActive && DriveTarget.Get() == OldTarget)
		{
			SubmitInteractionIntent(
				OldTarget,
				DrivePipelineTag,
				SingularisInteraction_Intent_Release,
				FSingularisInteractionValue()
			);
		}

		FlushDrive();

		OldTarget->OnInteractionSessionEvent.RemoveDynamic(
			this,
			&USingularisInteractorComponent::HandleTargetSessionEvent
		);

		if (IsLocalPlayerInteractor())
			OldTarget->SetHovered(false);
	}

	// 2) 新目标：绑定会话事件并激活悬浮
	if (IsValid(NewTarget))
	{
		NewTarget->OnInteractionSessionEvent.AddDynamic(
			this,
			&USingularisInteractorComponent::HandleTargetSessionEvent
		);

		if (IsLocalPlayerInteractor())
			NewTarget->SetHovered(true);
	}

	// 3) 通知驱动器刷新输入装配
	if (IsValid(InteractionDriver))
		InteractionDriver->OnTargetChanged(OldTarget, NewTarget);
}

bool USingularisInteractorComponent::BeginMirror(const USingularisInteractionSession* Archetype)
{
	// 1) 原型与驱动目标校验，派生镜像实例
	if (!IsValid(Archetype) || !DriveTarget.IsValid())
		return false;

	USingularisInteractionSession* MirrorInstance = DuplicateObject<USingularisInteractionSession>(Archetype, this);
	if (!IsValid(MirrorInstance))
		return false;

	MirrorInstance->BindSessionHost(this);

	// 2) 装配镜像状态（触发者与控制器按本组件宿主就地推导；会话启动置于末尾）
	MirrorSession = MirrorInstance;
	bMirrorActive = true;
	MirrorElapsed = 0.0f;

	MirrorContext = FSingularisInteractionSessionContext{};
	if (APawn* const OwnerPawn = Cast<APawn>(GetOwner()))
		MirrorContext.Instigator = OwnerPawn;
	else if (const AController* OwnerController = Cast<AController>(GetOwner()))
		MirrorContext.Instigator = OwnerController->GetPawn();
	MirrorContext.Controller = MirrorContext.Instigator ? MirrorContext.Instigator->GetController() : nullptr;
	MirrorContext.Avatar = DriveTarget->GetOwner();
	MirrorContext.Target = DriveTarget->GetOwner();
	MirrorContext.InteractionComponent = DriveTarget.Get();
	MirrorContext.PipelineTag = DrivePipelineTag;
	MirrorContext.Elapsed = 0.0f;

	// 3) 启动镜像会话（瞬时会话在此完成并结算）
	MirrorInstance->Begin(MirrorContext);
	return true;
}

void USingularisInteractorComponent::FeedMirrorIntent(const FSingularisInteractionIntent& Intent)
{
	// 1) 非镜像运行态静默吞并
	if (!bMirrorActive || !IsValid(MirrorSession))
		return;

	// 2) 更新会话时钟并委托会话摄入（会话可能结算并冲刷镜像，此后不得再触碰镜像状态）
	MirrorContext.Elapsed = MirrorElapsed;
	MirrorSession->ReceiveIntent(MirrorContext, Intent);
}

void USingularisInteractorComponent::MirrorTick(const float DeltaTime)
{
	// 1) 仅本地玩家交互者且镜像运行中时推进
	if (!IsLocalPlayerInteractor() || !bDriveActive || !bMirrorActive)
		return;

	// 2) 推进镜像会话（与逻辑端会话同一实现），累计时长并转发其发射
	MirrorElapsed += DeltaTime;
	MirrorContext.Elapsed = MirrorElapsed;
	if (IsValid(MirrorSession))
		MirrorSession->Tick(MirrorContext, DeltaTime);
}

void USingularisInteractorComponent::BroadcastInteractorSession(
	USingularisInteractionComponent* Target,
	const FGameplayTag& PipelineTag,
	const FGameplayTag& EventTag,
	// ReSharper disable once CppPassValueParameterByConstReference
	FSingularisInteractionValue Value,
	const FInstancedStruct& Payload
) const
{
	// 1) 组装会话事件（含身份上下文）：触发者与控制器按本组件宿主就地推导
	FSingularisInteractionSessionEvent Event;
	Event.EventTag = EventTag;
	Event.Value = Value;
	Event.Payload = Payload;
	if (APawn* const OwnerPawn = Cast<APawn>(GetOwner()))
		Event.Context.Instigator = OwnerPawn;
	else if (const AController* OwnerController = Cast<AController>(GetOwner()))
		Event.Context.Instigator = OwnerController->GetPawn();
	Event.Context.Controller = Event.Context.Instigator ? Event.Context.Instigator->GetController() : nullptr;
	Event.Context.Avatar = IsValid(Target) ? Target->GetOwner() : nullptr;
	Event.Context.Target = Event.Context.Avatar;
	Event.Context.InteractionComponent = Target;
	Event.Context.PipelineTag = PipelineTag;

	// 2) 广播本地流
	OnInteractorSessionEvent.Broadcast(Target, Event);
}

void USingularisInteractorComponent::EndMirror(const bool bCompleted, const FInstancedStruct& Payload)
{
	// 1) 非镜像运行态静默吞并
	if (!bMirrorActive)
		return;

	// 2) 捕获身份后冲刷镜像状态
	USingularisInteractionComponent* Target = DriveTarget.Get();
	const FGameplayTag PipelineTag = DrivePipelineTag;
	FlushDrive();

	// 3) 广播结算
	BroadcastInteractorSession(
		Target,
		PipelineTag,
		bCompleted ? SingularisInteraction_Event_Complete : SingularisInteraction_Event_Cancel,
		FSingularisInteractionValue(bCompleted ? 1.0f : 0.0f),
		Payload
	);
}

void USingularisInteractorComponent::FlushDrive()
{
	MirrorSession = nullptr;
	bMirrorActive = false;
	MirrorElapsed = 0.0f;

	bDriveActive = false;
	DriveTarget = nullptr;
	DrivePipelineTag = FGameplayTag{};
}
