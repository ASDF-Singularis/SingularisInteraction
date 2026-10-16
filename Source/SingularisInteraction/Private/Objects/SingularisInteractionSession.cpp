/* ====================================================================== *
 * SingularisInteractionSession.cpp                                       *
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

#include "Objects/SingularisInteractionSession.h"

#include "Interfaces/SingularisInteractionSessionHost.h"

void USingularisInteractionSession::EmitSessionEvent(
	const FGameplayTag EventTag,
	FSingularisInteractionValue Value,
	const FInstancedStruct& Payload
)
{
	// 1) 宿主失效时发射通道静默关闭
	UObject* const HostObject = SessionHostObject.Get();
	if (!IsValid(HostObject)) return;

	// 2) 经宿主接口契约路由
	ISingularisInteractionSessionHost::Execute_HostEmitSessionEvent(HostObject, this, EventTag, Value, Payload);
}

void USingularisInteractionSession::CompleteSession()
{
	// 1) 宿主失效时发射通道静默关闭
	UObject* const HostObject = SessionHostObject.Get();
	if (!IsValid(HostObject)) return;

	// 2) 经宿主接口契约路由
	ISingularisInteractionSessionHost::Execute_HostCompleteSession(HostObject, this);
}

void USingularisInteractionSession::CancelSession()
{
	// 1) 宿主失效时发射通道静默关闭
	UObject* const HostObject = SessionHostObject.Get();
	if (!IsValid(HostObject)) return;

	// 2) 经宿主接口契约路由
	ISingularisInteractionSessionHost::Execute_HostCancelSession(HostObject, this);
}

void USingularisInteractionSession::BindSessionHost(UObject* InSessionHostObject)
{
	SessionHostObject = InSessionHostObject;
}

bool USingularisInteractionSession::CanBegin_Implementation(const FSingularisInteractionSessionContext& Context) const
{
	return true;
}

void USingularisInteractionSession::Begin_Implementation(const FSingularisInteractionSessionContext& Context) {}

void USingularisInteractionSession::ReceiveIntent_Implementation(
	const FSingularisInteractionSessionContext& Context,
	const FSingularisInteractionIntent& Intent
) {}

void USingularisInteractionSession::Tick_Implementation(
	const FSingularisInteractionSessionContext& Context,
	const float DeltaTime
) {}

void USingularisInteractionSession::End_Implementation(const FSingularisInteractionSessionContext& Context) {}
