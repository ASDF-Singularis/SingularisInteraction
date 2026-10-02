/* ====================================================================== *
 * SingularisInteractionSubsystem.h                                      *
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

#pragma once

#include <CoreMinimal.h>
#include <Subsystems/WorldSubsystem.h>

#include "SingularisInteractionSubsystem.generated.h"

class USingularisInteractionComponent;

/**
 * 引力奇点交互子系统。
 *
 * 世界级交互映射表：登记可交互碰撞组件与其交互组件的对应关系，
 * 供查询器命中几何体后反查承载交互语义的组件。
 */
UCLASS(
	NotBlueprintable,
	BlueprintType,
	ClassGroup = ("Singularis"),
	meta = (DisplayName = "引力奇点交互子系统")
)
class SINGULARISINTERACTION_API USingularisInteractionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	/** 碰撞组件到交互组件的弱引用映射 */
	TMap<TWeakObjectPtr<UPrimitiveComponent>, TWeakObjectPtr<USingularisInteractionComponent>> Map{};

public:
	USingularisInteractionSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * 登记碰撞组件与交互组件的映射关系。
	 *
	 * 重复登记同一碰撞组件时覆盖既有映射。
	 *
	 * @param PrimitiveComponent 可交互的碰撞组件。
	 * @param InteractionComponent 承载交互语义的交互组件。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互子系统",
		meta = (DisplayName = "注册映射")
	)
	void RegisterMapping(
		UPrimitiveComponent* PrimitiveComponent,
		USingularisInteractionComponent* InteractionComponent
	);

	/**
	 * 注销碰撞组件的映射关系。
	 *
	 * @param PrimitiveComponent 待注销的碰撞组件。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互子系统",
		meta = (DisplayName = "注销映射")
	)
	void UnregisterMapping(UPrimitiveComponent* PrimitiveComponent);

	/**
	 * 反查碰撞组件登记的交互组件。
	 *
	 * @param PrimitiveComponent 命中的碰撞组件。
	 * @return 已登记的交互组件；未登记或弱引用失效时返回 nullptr。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互子系统",
		meta = (DisplayName = "映射组件")
	)
	USingularisInteractionComponent* MappingComponent(UPrimitiveComponent* PrimitiveComponent);
};
