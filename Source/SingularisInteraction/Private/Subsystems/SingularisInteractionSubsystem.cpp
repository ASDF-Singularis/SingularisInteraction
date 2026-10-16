/* ====================================================================== *
 * SingularisInteractionSubsystem.cpp                                     *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/07/25 | Author: TrifingZW                                *
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

#include "Subsystems/SingularisInteractionSubsystem.h"

#include <Components/PrimitiveComponent.h>
#include <GameFramework/Actor.h>

#include "SingularisInteraction.h"
#include "Components/SingularisInteractionComponent.h"

USingularisInteractionSubsystem::USingularisInteractionSubsystem() {}

void USingularisInteractionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USingularisInteractionSubsystem::Deinitialize() { Super::Deinitialize(); }

bool USingularisInteractionSubsystem::SubmitInteractionIntent(
	AActor* Interactor,
	USingularisInteractionComponent* TargetInteractionComponent,
	const FGameplayTag PipelineTag,
	const FSingularisInteractionIntent Intent
)
{
	// 1) 零信任校验
	if (!IsValid(Interactor) || !IsValid(TargetInteractionComponent))
	{
		UE_LOG(LogSingularisInteraction, Warning, TEXT("SubmitInteractionIntent：交互者或目标交互组件无效，拒绝请求"));
		return false;
	}

	if (!PipelineTag.IsValid() || !Intent.IntentTag.IsValid())
		return false;

	// 2) 转交目标组件（会话唯一结算入口）
	TargetInteractionComponent->ReceiveInteractionIntent(Interactor, PipelineTag, Intent);
	return true;
}

void USingularisInteractionSubsystem::RegisterMapping(
	UPrimitiveComponent* PrimitiveComponent,
	USingularisInteractionComponent* InteractionComponent
)
{
	// 1) 零信任校验
	if (!IsValid(PrimitiveComponent) || !IsValid(InteractionComponent))
		return;

	// 2) 登记映射（重复登记覆盖既有映射）
	Map.Add(PrimitiveComponent, InteractionComponent);
}

void USingularisInteractionSubsystem::UnregisterMapping(UPrimitiveComponent* PrimitiveComponent)
{
	// 1) 注销映射（未登记时幂等返回）
	if (!IsValid(PrimitiveComponent))
		return;

	Map.Remove(PrimitiveComponent);
}

USingularisInteractionComponent* USingularisInteractionSubsystem::MappingComponent(
	UPrimitiveComponent* PrimitiveComponent
)
{
	// 1) 零信任校验
	if (!IsValid(PrimitiveComponent))
		return nullptr;

	// 2) 哈希查找后校验弱引用有效性
	const TWeakObjectPtr<USingularisInteractionComponent>* Found = Map.Find(PrimitiveComponent);
	return Found && Found->IsValid() ? Found->Get() : nullptr;
}

void USingularisInteractionSubsystem::NotifyWorldSessionEvent(
	USingularisInteractionComponent* Target,
	const FSingularisInteractionSessionEvent& Event
)
{
	// 1) 广播世界级会话事件（全局观察 / 遥测）
	OnWorldInteractionSessionEvent.Broadcast(Target, Event);
}
