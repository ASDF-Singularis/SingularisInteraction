/* ====================================================================== *
 * SingularisInteractionResponse.cpp                                      *
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

#include "Objects/SingularisInteractionResponse.h"

#include <Engine/World.h>

#include "Types/SingularisInteractionEventHandlerRegistry.h"

UWorld* USingularisInteractionResponse::GetWorld() const
{
	// 1) 排除 CDO：防止在编辑器启动或序列化时获取错误的上下文
	if (HasAnyFlags(RF_ClassDefaultObject)) return nullptr;

	// 2) 通过 Outer 链（交互组件 → Owner Actor）获取 WorldContext
	if (const UObject* Outer = GetOuter()) return Outer->GetWorld();

	return Super::GetWorld();
}

void USingularisInteractionResponse::DispatchSessionEvent(const FSingularisInteractionSessionEvent& Event)
{
	// 1) 进入分派帧：写入当前事件，声明式处理器与蓝图节点经此读取负载（嵌套分派由保存/恢复隔离）
	const FSingularisInteractionSessionEvent PreviousEvent = CurrentSessionEvent;
	CurrentSessionEvent = Event;

	// 2) 声明式处理器：宏登记与蓝图节点，沿类层级自派生向基类匹配
	DispatchDeclaredHandlers(Event);

	// 3) 退出分派帧：恢复上一帧事件
	CurrentSessionEvent = PreviousEvent;
}

FSingularisInteractionValue USingularisInteractionResponse::GetCurrentValue() const
{
	// 分派帧外返回零值：不向帧外调用方暴露残留负载
	return CurrentSessionEvent.Value;
}

FInstancedStruct USingularisInteractionResponse::GetCurrentPayload() const
{
	// 分派帧外返回空结构：与量值同一约定
	return CurrentSessionEvent.Payload;
}

FSingularisInteractionSessionContext USingularisInteractionResponse::GetCurrentContext() const
{
	// 分派帧外返回空上下文：与量值、瞬态负荷同一约定
	return CurrentSessionEvent.Context;
}

bool USingularisInteractionResponse::MatchesCurrentSessionEvent(const FGameplayTag& EventTag) const
{
	// 零信任：订阅标签可能未配置，无效则不命中
	return EventTag.IsValid() && CurrentSessionEvent.EventTag.MatchesTag(EventTag);
}

void USingularisInteractionResponse::DispatchDeclaredHandlers(const FSingularisInteractionSessionEvent& Event)
{
	// 1) 自派生向基类逐类取声明，派生类先于基类执行
	for (const UClass* CurrentClass = GetClass(); CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
	{
		// 2) C++ 宏登记：标签层级匹配后按登记顺序调用
		if (const TArray<FSingularisInteractionEventHandler>* Handlers =
			FSingularisInteractionEventHandlerRegistry::Get().FindHandlers(CurrentClass))
		{
			for (const FSingularisInteractionEventHandler& Entry : *Handlers)
			{
				if (!Entry.Handler || !Entry.EventTag.IsValid() || !Event.EventTag.MatchesTag(Entry.EventTag))
					continue;

				(this->*Entry.Handler)(Event.Context, Event.Value, Event.Payload);
			}
		}

		// 3) 蓝图节点声明：按名解析处理器函数并直接调用；处理器约定无入参、无返回值
		const USingularisInteractionResponse* const ClassDefault =
			Cast<USingularisInteractionResponse>(CurrentClass->GetDefaultObject());
		if (!ClassDefault)
			continue;

		for (const auto& [SubscribedTag, HandlerName] : ClassDefault->DeclaredSessionEventHandlers)
		{
			if (!SubscribedTag.IsValid() || !Event.EventTag.MatchesTag(SubscribedTag))
				continue;

			UFunction* const Handler = CurrentClass->FindFunctionByName(HandlerName);
			if (!Handler || Handler->NumParms != 0 || Handler->ReturnValueOffset != MAX_uint16)
				continue;

			ProcessEvent(Handler, nullptr);
		}
	}
}
