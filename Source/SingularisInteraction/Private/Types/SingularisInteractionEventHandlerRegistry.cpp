/* ====================================================================== *
 * SingularisInteractionEventHandlerRegistry.cpp                          *
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

#include "Types/SingularisInteractionEventHandlerRegistry.h"

#include "SingularisInteraction.h"

FSingularisInteractionEventHandlerRegistry& FSingularisInteractionEventHandlerRegistry::Get()
{
	static FSingularisInteractionEventHandlerRegistry Instance;
	return Instance;
}

void FSingularisInteractionEventHandlerRegistry::RegisterHandler(
	UClass* (*ResponseClassResolver)(),
	FGameplayTag (*EventTagResolver)(),
	FSingularisInteractionHandlerPtr Handler
)
{
	// 1) 零信任校验：解析器或处理器缺失的请求直接丢弃
	if (!ResponseClassResolver || !EventTagResolver || !Handler)
		return;

	// 2) 同键（类解析器 + 标签解析器 + 处理器）幂等吞并，避免模块重载重复提交
	const bool bPending = PendingRegistrations.ContainsByPredicate(
		[ResponseClassResolver, EventTagResolver, Handler](
		const FSingularisInteractionHandlerRegistration& Registration
	)
		{
			return Registration.ResponseClassResolver == ResponseClassResolver
				&& Registration.EventTagResolver == EventTagResolver
				&& Registration.Handler == Handler;
		}
	);
	if (bPending)
		return;

	PendingRegistrations.Add({ResponseClassResolver, EventTagResolver, Handler});
}

const TArray<FSingularisInteractionEventHandler>* FSingularisInteractionEventHandlerRegistry::FindHandlers(
	const UClass* ResponseClass
)
{
	// 1) 首次查找时解析暂存登记：此刻引擎与标签常量均已就绪
	if (!bRegistrationsResolved)
		ResolvePendingRegistrations();

	return ResponseClass ? Handlers.Find(ResponseClass) : nullptr;
}

void FSingularisInteractionEventHandlerRegistry::ResolvePendingRegistrations()
{
	bRegistrationsResolved = true;

	for (const FSingularisInteractionHandlerRegistration& Registration : PendingRegistrations)
	{
		// 1) 解析声明并零信任校验：解析后仍无效的声明丢弃并告警
		UClass* const ResponseClass = Registration.ResponseClassResolver();
		const FGameplayTag EventTag = Registration.EventTagResolver();
		if (!IsValid(ResponseClass) || !EventTag.IsValid())
		{
			UE_LOG(
				LogSingularisInteraction,
				Warning,
				TEXT("ResolvePendingRegistrations：忽略无效声明（类=%s，标签=%s）"),
				*GetNameSafe(ResponseClass),
				*EventTag.ToString()
			);
			continue;
		}

		// 2) 同键（类 + 标签 + 处理器）幂等合并
		TArray<FSingularisInteractionEventHandler>& Entries = Handlers.FindOrAdd(ResponseClass);
		const bool bRegistered = Entries.ContainsByPredicate(
			[&EventTag, &Registration](const FSingularisInteractionEventHandler& Entry)
			{
				return Entry.EventTag == EventTag && Entry.Handler == Registration.Handler;
			}
		);
		if (bRegistered)
			continue;

		Entries.Add({EventTag, Registration.Handler});
	}

	PendingRegistrations.Reset();
}
