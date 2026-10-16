/* ====================================================================== *
 * SingularisInteractionEventHandlerRegistry.h                            *
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
#include <GameplayTagContainer.h>

#include "Types/SingularisInteractionSessionType.h"

class USingularisInteractionResponse;

/**
 * 引力奇点交互事件处理器条目。
 */
struct FSingularisInteractionEventHandler
{
	/** 订阅的事件标签（解析后固化，层级匹配） */
	FGameplayTag EventTag{};

	/** 处理器成员函数指针（入参：量值、瞬态负荷） */
	void (USingularisInteractionResponse::*Handler)(
		const FSingularisInteractionValue&,
		const FInstancedStruct&
	) = nullptr;
};

/**
 * 引力奇点交互事件处理器登记请求。
 *
 * 声明以解析器形式提交：响应类与事件标签都在首次查找时才求值，
 * 因为提交发生在静态初始化期，此时触碰 UObject 系统或尚未初始化的标签常量都不安全。
 */
struct FSingularisInteractionHandlerRegistration
{
	/** 响应类解析器 */
	UClass* (*ResponseClassResolver)() = nullptr;

	/** 订阅的事件标签解析器 */
	FGameplayTag (*EventTagResolver)() = nullptr;

	/** 处理器成员函数指针（入参：量值、瞬态负荷） */
	void (USingularisInteractionResponse::*Handler)(
		const FSingularisInteractionValue&,
		const FInstancedStruct&
	) = nullptr;
};

/**
 * 引力奇点交互事件处理器注册表。
 *
 * 声明式事件分派的运行时载体：`SINGULARIS_RESPONSE_EVENT` 在模块加载期提交登记请求，
 * 首次查找时解析为（响应类 / 事件标签 / 处理器）并按类归表；
 * 响应基类沿类层级自派生向基类匹配并调用。
 * 仅保存声明元数据，不持有响应实例；访问限于游戏线程。
 */
class SINGULARISINTERACTION_API FSingularisInteractionEventHandlerRegistry
{
public:
	/**
	 * 取全局单例。
	 *
	 * @return 注册表单例。
	 */
	static FSingularisInteractionEventHandlerRegistry& Get();

	/**
	 * 提交处理器登记请求（模块加载期调用；仅暂存，首次查找时解析）。
	 *
	 * @param ResponseClassResolver 响应类解析器。
	 * @param EventTagResolver 订阅的事件标签解析器。
	 * @param Handler 处理器成员函数指针。
	 */
	void RegisterHandler(
		UClass* (*ResponseClassResolver)(),
		FGameplayTag (*EventTagResolver)(),
		void (USingularisInteractionResponse::*Handler)(const FSingularisInteractionValue&, const FInstancedStruct&)
	);

	/**
	 * 查询指定响应类登记的处理器。
	 *
	 * @param ResponseClass 响应类。
	 * @return 处理器数组；未登记时返回 nullptr。
	 */
	const TArray<FSingularisInteractionEventHandler>* FindHandlers(const UClass* ResponseClass);

private:
	/**
	 * 解析暂存登记并归表。
	 *
	 * 幂等；解析后仍无效的声明丢弃并告警，同键（类 + 标签 + 处理器）合并。
	 */
	void ResolvePendingRegistrations();

	/** 待解析的登记请求 */
	TArray<FSingularisInteractionHandlerRegistration> PendingRegistrations{};

	/** 响应类 → 处理器集 */
	TMap<UClass*, TArray<FSingularisInteractionEventHandler>> Handlers{};

	/** 暂存登记是否已解析 */
	bool bRegistrationsResolved = false;
};
