/* ====================================================================== *
 * SingularisInteractionSubsystem.cpp                                      *
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

#include "SingularisInteraction.h"
#include "Components/SingularisInteractionComponent.h"

USingularisInteractionSubsystem::USingularisInteractionSubsystem() {}

void USingularisInteractionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] Initialize：交互映射子系统初始化完成"),
		*GetNameSafe(this)
	);
}

void USingularisInteractionSubsystem::Deinitialize()
{
	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] Deinitialize：交互映射子系统卸载，注销 %d 条映射"),
		*GetNameSafe(this),
		Map.Num()
	);

	Map.Empty();

	Super::Deinitialize();
}

void USingularisInteractionSubsystem::RegisterMapping(
	UPrimitiveComponent* PrimitiveComponent,
	USingularisInteractionComponent* InteractionComponent
)
{
	// 1) 空指针守卫
	if (!IsValid(PrimitiveComponent) || !IsValid(InteractionComponent))
	{
		UE_LOG(
			LogSingularisInteraction,
			Warning,
			TEXT("[%s] RegisterMapping：入参非法（碰撞组件 %s，交互组件 %s）"),
			*GetNameSafe(this),
			*GetNameSafe(PrimitiveComponent),
			*GetNameSafe(InteractionComponent)
		);
		return;
	}

	// 2) 登记映射，重复登记时覆盖既有映射
	Map.Add(PrimitiveComponent, InteractionComponent);
}

void USingularisInteractionSubsystem::UnregisterMapping(UPrimitiveComponent* PrimitiveComponent)
{
	// 1) 空指针守卫
	if (!IsValid(PrimitiveComponent))
		return;

	// 2) 移除映射，未登记时幂等返回
	Map.Remove(PrimitiveComponent);
}

USingularisInteractionComponent* USingularisInteractionSubsystem::MappingComponent(
	UPrimitiveComponent* PrimitiveComponent
)
{
	// 1) 空指针守卫
	if (!IsValid(PrimitiveComponent))
		return nullptr;

	// 2) 哈希查找 → 弱引用有效性检查 → 解引用
	if (const TWeakObjectPtr<USingularisInteractionComponent>* Found = Map.Find(PrimitiveComponent))
		if (Found->IsValid()) return Found->Get();

	return nullptr;
}
