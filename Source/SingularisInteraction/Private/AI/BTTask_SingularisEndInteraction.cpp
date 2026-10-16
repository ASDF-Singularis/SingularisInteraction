/* ====================================================================== *
 * BTTask_SingularisEndInteraction.cpp                                    *
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

#include "AI/BTTask_SingularisEndInteraction.h"

#include <AIController.h>
#include <BehaviorTree/BehaviorTreeComponent.h>
#include <BehaviorTree/BlackboardComponent.h>
#include <GameFramework/Pawn.h>

#include "Components/SingularisInteractionComponent.h"
#include "Components/SingularisInteractorComponent.h"

UBTTask_SingularisEndInteraction::UBTTask_SingularisEndInteraction() { NodeName = TEXT("Singularis 撤回交互"); }

EBTNodeResult::Type UBTTask_SingularisEndInteraction::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	// 1) 解析交互者组件（AIController 优先，其次其 Pawn）
	const AAIController* AIController = Cast<AAIController>(OwnerComp.GetOwner());
	if (!IsValid(AIController))
		return EBTNodeResult::Failed;

	USingularisInteractorComponent* Interactor =
		AIController->FindComponentByClass<USingularisInteractorComponent>();
	if (!IsValid(Interactor) && IsValid(AIController->GetPawn()))
		Interactor = AIController->GetPawn()->FindComponentByClass<USingularisInteractorComponent>();
	if (!IsValid(Interactor))
		return EBTNodeResult::Failed;

	// 2) 读取黑板目标对象（交互组件或 Actor）
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!IsValid(Blackboard) || TargetKey.SelectedKeyName.IsNone())
		return EBTNodeResult::Failed;

	UObject* TargetObject = Blackboard->GetValueAsObject(TargetKey.SelectedKeyName);
	USingularisInteractionComponent* Target = Cast<USingularisInteractionComponent>(TargetObject);
	if (!IsValid(Target))
	{
		if (const AActor* TargetActor = Cast<AActor>(TargetObject))
			Target = TargetActor->FindComponentByClass<USingularisInteractionComponent>();
	}
	if (!IsValid(Target))
		return EBTNodeResult::Failed;

	// 3) 提交撤回意图
	return Interactor->EndInteraction(Target, PipelineTag) ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

FString UBTTask_SingularisEndInteraction::GetStaticDescription() const
{
	// 1) 展示标签与目标键，便于行为树审阅
	return FString::Printf(TEXT("管线标签: %s\n目标键: %s"), *PipelineTag.ToString(), *TargetKey.SelectedKeyName.ToString());
}
