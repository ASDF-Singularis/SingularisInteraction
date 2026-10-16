/* ====================================================================== *
 * BTTask_SingularisEmitValue.cpp                                         *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/10/13 | Author: TrifingZW                                *
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

#include "AI/BTTask_SingularisEmitValue.h"

#include <AIController.h>
#include <BehaviorTree/BehaviorTreeComponent.h>
#include <BehaviorTree/BlackboardComponent.h>
#include <GameFramework/Pawn.h>

#include "SingularisInteraction.h"
#include "Components/SingularisInteractionComponent.h"
#include "Components/SingularisInteractorComponent.h"
#include "Types/SingularisInteractionGameplayTags.h"

UBTTask_SingularisEmitValue::UBTTask_SingularisEmitValue()
{
	NodeName = TEXT("Singularis 供值交互");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_SingularisEmitValue::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	// 1) 解析交互者与目标
	USingularisInteractorComponent* Interactor = ResolveInteractorComponent(OwnerComp);
	USingularisInteractionComponent* Target = ResolveTargetComponent(OwnerComp);
	if (!IsValid(Interactor) || !IsValid(Target))
	{
		UE_LOG(LogSingularisInteraction, Warning, TEXT("[%s] 供值交互：交互者组件或目标解析失败"), *NodeName);
		return EBTNodeResult::Failed;
	}

	// 2) 先绑定观察再提交意图（避免错失瞬时结算）
	ObservedInteractor = Interactor;
	ActiveTarget = Target;
	OwnerComponent = &OwnerComp;
	ElapsedSeconds = 0.0f;
	EmitAccumulator = 0.0f;
	bLatentFinished = false;
	Interactor->OnInteractorSessionEvent.AddDynamic(this, &UBTTask_SingularisEmitValue::HandleInteractorSessionEvent);

	// 3) 建立会话（可选）；失败即解绑并失败
	if (bEstablishSession && !Interactor->BeginInteraction(Target, PipelineTag))
	{
		Interactor->OnInteractorSessionEvent.RemoveDynamic(
			this,
			&UBTTask_SingularisEmitValue::HandleInteractorSessionEvent
		);
		ObservedInteractor = nullptr;
		ActiveTarget = nullptr;
		OwnerComponent = nullptr;
		return EBTNodeResult::Failed;
	}

	// 4) 提交首次供值并潜伏
	EmitValue(OwnerComp);

	return EBTNodeResult::InProgress;
}

void UBTTask_SingularisEmitValue::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	const float DeltaSeconds
)
{
	// 1) 交互者或目标失效即失败
	if (!ObservedInteractor.IsValid() || !ActiveTarget.IsValid())
	{
		FinishLatent(OwnerComp, false);
		return;
	}

	// 2) 供值节流：间隔为零逐帧提交，否则累计到达即提交
	EmitAccumulator += DeltaSeconds;
	if (EmitInterval <= 0.0f || EmitAccumulator >= EmitInterval)
	{
		EmitValue(OwnerComp);
		EmitAccumulator = 0.0f;
	}

	// 3) 时长耗尽：撤回并完成（0 为持续至会话结算）
	if (DurationSeconds <= 0.0f)
		return;

	ElapsedSeconds += DeltaSeconds;
	if (ElapsedSeconds >= DurationSeconds)
	{
		ObservedInteractor->EndInteraction(ActiveTarget.Get(), PipelineTag);
		FinishLatent(OwnerComp, true);
	}
}

EBTNodeResult::Type UBTTask_SingularisEmitValue::AbortTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	// 1) 由本任务建立会话时撤回，随后解绑观察
	if (ObservedInteractor.IsValid())
	{
		if (bEstablishSession)
			ObservedInteractor->EndInteraction(ActiveTarget.Get(), PipelineTag);

		ObservedInteractor->OnInteractorSessionEvent.RemoveDynamic(
			this,
			&UBTTask_SingularisEmitValue::HandleInteractorSessionEvent
		);
	}

	ObservedInteractor = nullptr;
	ActiveTarget = nullptr;
	OwnerComponent = nullptr;

	return EBTNodeResult::Aborted;
}

FString UBTTask_SingularisEmitValue::GetStaticDescription() const
{
	// 1) 展示标签、目标键与供值来源，便于行为树审阅
	const FString ValueSource = ValueKey.SelectedKeyName.IsNone()
		                            ? FString::Printf(TEXT("%.2f"), Value)
		                            : ValueKey.SelectedKeyName.ToString();

	return FString::Printf(
		TEXT("管线标签: %s\n目标键: %s\n供值: %s"),
		*PipelineTag.ToString(),
		*TargetKey.SelectedKeyName.ToString(),
		*ValueSource
	);
}

void UBTTask_SingularisEmitValue::HandleInteractorSessionEvent(
	USingularisInteractionComponent* Target,
	FSingularisInteractionSessionEvent Event
)
{
	// 1) 仅关注本任务会话（交互者与管线标签一致）
	if (!ObservedInteractor.IsValid() || Event.Context.Interactor != ObservedInteractor->GetOwner())
		return;
	if (Event.Context.PipelineTag != PipelineTag)
		return;

	// 2) 结算判据：达成为成功、中止为失败
	if (Event.EventTag != SingularisInteraction_Event_Complete && Event.EventTag != SingularisInteraction_Event_Cancel)
		return;

	if (OwnerComponent.IsValid())
		FinishLatent(*OwnerComponent.Get(), Event.EventTag == SingularisInteraction_Event_Complete);
}

USingularisInteractorComponent* UBTTask_SingularisEmitValue::ResolveInteractorComponent(
	UBehaviorTreeComponent& OwnerComp
) const
{
	// 1) AIController 优先，其次其 Pawn
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetOwner());
	if (!IsValid(AIController))
		return nullptr;

	if (USingularisInteractorComponent* Interactor =
		AIController->FindComponentByClass<USingularisInteractorComponent>())
		return Interactor;

	if (const APawn* OwnerPawn = AIController->GetPawn())
		return OwnerPawn->FindComponentByClass<USingularisInteractorComponent>();

	return nullptr;
}

USingularisInteractionComponent* UBTTask_SingularisEmitValue::ResolveTargetComponent(
	UBehaviorTreeComponent& OwnerComp
) const
{
	// 1) 读取黑板目标对象（交互组件或 Actor）
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!IsValid(Blackboard) || TargetKey.SelectedKeyName.IsNone())
		return nullptr;

	UObject* TargetObject = Blackboard->GetValueAsObject(TargetKey.SelectedKeyName);
	if (const auto* TargetComponent = Cast<USingularisInteractionComponent>(TargetObject))
		return const_cast<USingularisInteractionComponent*>(TargetComponent);

	if (AActor* TargetActor = Cast<AActor>(TargetObject))
		return TargetActor->FindComponentByClass<USingularisInteractionComponent>();

	return nullptr;
}

float UBTTask_SingularisEmitValue::ResolveValue(UBehaviorTreeComponent& OwnerComp) const
{
	// 1) 黑板键优先，未选择时回退常量量值
	if (!ValueKey.SelectedKeyName.IsNone())
	{
		if (const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
			return Blackboard->GetValueAsFloat(ValueKey.SelectedKeyName);
	}

	return Value;
}

void UBTTask_SingularisEmitValue::EmitValue(UBehaviorTreeComponent& OwnerComp)
{
	// 1) 经交互者组件单一提交入口提交连续值意图
	if (!ObservedInteractor.IsValid() || !ActiveTarget.IsValid())
		return;

	ObservedInteractor->SubmitInteractionIntent(
		ActiveTarget.Get(),
		PipelineTag,
		SingularisInteraction_Intent_Value,
		FSingularisInteractionValue(ResolveValue(OwnerComp))
	);
}

void UBTTask_SingularisEmitValue::FinishLatent(UBehaviorTreeComponent& OwnerComp, const bool bSucceeded)
{
	// 1) 重复结算保护：潜伏任务只结算一次
	if (bLatentFinished)
		return;
	bLatentFinished = true;

	// 2) 解绑观察
	if (ObservedInteractor.IsValid())
	{
		ObservedInteractor->OnInteractorSessionEvent.RemoveDynamic(
			this,
			&UBTTask_SingularisEmitValue::HandleInteractorSessionEvent
		);
	}

	// 3) 清理状态并完成潜伏
	ObservedInteractor = nullptr;
	ActiveTarget = nullptr;
	OwnerComponent = nullptr;

	FinishLatentTask(OwnerComp, bSucceeded ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
}
