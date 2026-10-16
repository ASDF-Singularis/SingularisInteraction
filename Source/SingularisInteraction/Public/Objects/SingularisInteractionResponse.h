/* ====================================================================== *
 * SingularisInteractionResponse.h                                        *
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

#pragma once

#include <CoreMinimal.h>
#include <UObject/Object.h>

#include "Types/SingularisInteractionSessionType.h"
#include "SingularisInteractionResponse.generated.h"

/**
 * 引力奇点交互响应。
 *
 * 会话事件的副作用执行单元：以单一入口 `DispatchSessionEvent` 消费统一事件流，
 * 事件经声明式分派落地（C++ 用 `SINGULARIS_RESPONSE_EVENT` 宏、蓝图用事件图内的会话事件节点，
 * 框架直接调用被声明的处理器函数），核心事件之外经标签开放扩展；事件处理不存在其他入口。
 * 处理器入参为会话上下文、量值与瞬态负荷三通道，不经框架公共事件结构传递，负载形态由发射会话自定。
 * 响应只消费会话语义，不感知事件如何产生；表现由框架事件流承担，响应永不自行组网。
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, CollapseCategories)
class SINGULARISINTERACTION_API USingularisInteractionResponse : public UObject
{
	GENERATED_BODY()

public:
#pragma region UObject Interface

	virtual UWorld* GetWorld() const override;

#pragma endregion

private:
#pragma region State

	/** 分派帧的当前会话事件（仅 DispatchSessionEvent 期间有效） */
	FSingularisInteractionSessionEvent CurrentSessionEvent{};

	/**
	 * 蓝图侧声明的会话事件处理器（事件标签 → 处理器函数名）。
	 *
	 * 由编辑器编译 hook 扫描蓝图事件图内的会话事件节点回填，随蓝图资产序列化并参与烘焙，
	 * 打包版本运行时无需编辑器即可读取；同标签重复声明只保留首个。
	 */
	UPROPERTY()
	TMap<FGameplayTag, FName> DeclaredSessionEventHandlers{};

	/** 编辑器编译 hook 需回填上方声明表 */
	friend class FSingularisInteractionBlueprintCompileHook;

#pragma endregion

public:
#pragma region API

	/**
	 * 会话事件分派入口（交互组件唯一调用点、事件处理的唯一入口）。
	 *
	 * 进入分派帧写入当前会话事件，执行声明式登记的处理器（C++ 宏 + 蓝图节点），再退出分派帧。
	 *
	 * @param Event 会话事件，含事件标签、量值、瞬态负荷与会话上下文。
	 */
	void DispatchSessionEvent(const FSingularisInteractionSessionEvent& Event);

	/**
	 * 取分派帧的当前量值。
	 *
	 * 仅在 `DispatchSessionEvent` 期间有效；声明式处理器与蓝图节点经此读取本帧量值。
	 *
	 * @return 当前量值；分派帧外为零值。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互响应|API",
		meta = (DisplayName = "GetCurrentValue")
	)
	FSingularisInteractionValue GetCurrentValue() const;

	/**
	 * 取分派帧的当前瞬态负荷。
	 *
	 * 仅在 `DispatchSessionEvent` 期间有效；结构类型由发射会话自定，读取方按类型取用。
	 *
	 * @return 当前瞬态负荷；分派帧外为空结构。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互响应|API",
		meta = (DisplayName = "GetCurrentPayload")
	)
	FInstancedStruct GetCurrentPayload() const;

	/**
	 * 取分派帧的当前会话上下文。
	 *
	 * 仅在 `DispatchSessionEvent` 期间有效；给出本次事件的触发者、控制器、宿主、目标、管线标签与累计时长。
	 *
	 * @return 当前会话上下文；分派帧外为空上下文。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互响应|API",
		meta = (DisplayName = "GetCurrentContext")
	)
	FSingularisInteractionSessionContext GetCurrentContext() const;

	/**
	 * 判定当前会话事件是否命中订阅标签（层级匹配）。
	 *
	 * @param EventTag 订阅的事件标签；事件标签本身或其子标签均命中。
	 * @return 命中时返回 true；订阅标签无效或处于分派帧外时返回 false。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互响应|API",
		meta = (DisplayName = "MatchesCurrentSessionEvent")
	)
	bool MatchesCurrentSessionEvent(const FGameplayTag& EventTag) const;

#pragma endregion

private:
#pragma region Internal Function

	/**
	 * 执行本类层级上声明式登记的处理器（C++ 宏登记 + 蓝图节点声明）。
	 *
	 * 沿 `GetClass()` 自派生向基类逐类匹配，标签采用层级匹配（订阅标签的后代亦可命中）；
	 * 类内先执行宏登记的处理器（按登记顺序），再执行蓝图声明的处理器（无顺序保证）。
	 *
	 * @param Event 会话事件。
	 */
	void DispatchDeclaredHandlers(const FSingularisInteractionSessionEvent& Event);

#pragma endregion
};
