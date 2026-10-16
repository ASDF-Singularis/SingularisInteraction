/* ====================================================================== *
 * SingularisInteractionDeclarativeMacros.h                               *
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

#include "Objects/SingularisInteractionResponse.h"
#include "Types/SingularisInteractionEventHandlerRegistry.h"

/**
 * 声明式登记会话事件处理器
 * 必须放置于类体（系列惯例置于 `public` 区域），替代 `Switch on GameplayTag` 式手写分派。
 *
 * 展开：
 *   1) 注册器结构体 F[HandlerName]EventRegistrar，构造期把（类解析器 /
 *      标签解析器 / &[HandlerName]）提交给 FSingularisInteractionEventHandlerRegistry；
 *   2) 静态注册实例，模块加载期提交登记请求（首次查找时解析）。
 *
 * 登记只提交解析器，响应类与事件标签均在首次查找时求值：
 * 提交发生在静态初始化期，类（UObject 系统）与标签常量（跨编译单元）都尚未就绪。
 *
 * 处理器签名为 `void HandlerName(const FSingularisInteractionValue& Value, const FInstancedStruct& Payload)`，
 * 可定义于类体内或类体外、可先于本宏声明；类内任意访问级别均可。
 * 分派由响应基类的 `DispatchSessionEvent` 驱动（沿类层级自派生向基类匹配，标签层级匹配）。
 *
 * @param EventTag    事件标签常量（如 `SingularisInteraction_Event_Complete`）
 * @param HandlerName 处理器成员函数名
 */
#define SINGULARIS_RESPONSE_EVENT(EventTag, HandlerName) \
	struct F##HandlerName##EventRegistrar \
	{ \
		F##HandlerName##EventRegistrar() \
		{ \
			FSingularisInteractionEventHandlerRegistry::Get().RegisterHandler( \
				&F##HandlerName##EventRegistrar::ResolveResponseClass, \
				&F##HandlerName##EventRegistrar::ResolveEventTag, \
				&HandlerName \
			); \
		} \
		static UClass* ResolveResponseClass() { return StaticClass(); } \
		static FGameplayTag ResolveEventTag() { return EventTag; } \
	}; \
	static inline F##HandlerName##EventRegistrar HandlerName##EventRegistrar{};
