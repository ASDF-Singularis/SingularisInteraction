/* ====================================================================== *
 * SingularisInteractionStrategy.cpp                                      *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/01/21 | Author: TrifingZW                                *
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

#include "Objects/SingularisInteractionStrategy.h"

#include <Engine/NetDriver.h>
#include <GameFramework/Actor.h>

UWorld* USingularisInteractionStrategy::GetWorld() const
{
	// 1) 排除 CDO：防止在编辑器启动或序列化时获取错误的上下文
	if (HasAnyFlags(RF_ClassDefaultObject)) return nullptr;

	// 2) 通过 Outer 链（AbilityComponent → OwnerActor）获取 WorldContext
	if (const UObject* Outer = GetOuter()) return Outer->GetWorld();

	return Super::GetWorld();
}

void USingularisInteractionStrategy::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool USingularisInteractionStrategy::IsSupportedForNetworking() const
{
	return true;
}

int32 USingularisInteractionStrategy::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	// 1) CDO 不支持网络调用，直接返回 Local
	if (HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking()) return FunctionCallspace::Local;

	// 2) 通过 Outer（AbilityComponent）链式委托，由 UActorComponent::GetFunctionCallspace 再委托至 Owner Actor
	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool USingularisInteractionStrategy::CallRemoteFunction(
	UFunction* Function,
	void* Parms,
	FOutParmRec* OutParms,
	FFrame* Stack
)
{
	// 1) CDO 不支持网络调用
	if (HasAnyFlags(RF_ClassDefaultObject)) return false;

	// 2) 沿 Outer 链查找 Owner Actor，通过其 NetDriver 转发 RPC
	AActor* OwnerActor = GetTypedOuter<AActor>();
	if (!IsValid(OwnerActor)) return false;

	UNetDriver* NetDriver = OwnerActor->GetNetDriver();
	if (!IsValid(NetDriver)) return false;

	// 3) 将 this（子对象）作为最后一个参数传入，使 NetDriver 正确路由子对象上的 RPC
	NetDriver->ProcessRemoteFunction(OwnerActor, Function, Parms, OutParms, Stack, this);

	return true;
}

bool USingularisInteractionStrategy::CanExecute_Implementation(
	const FSingularisInteractionStrategyContext& Context
) const
{
	return true;
}

void USingularisInteractionStrategy::Execute_Implementation(const FSingularisInteractionStrategyContext& Context) {}
