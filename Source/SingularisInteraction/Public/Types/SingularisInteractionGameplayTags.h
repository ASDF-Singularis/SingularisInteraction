/* ====================================================================== *
 * SingularisInteractionGameplayTags.h                                    *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/01/22 | Author: TrifingZW                                *
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
#include <NativeGameplayTags.h>

/** 交互域根标签 */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SingularisInteraction);

/** 意图域根标签：控制端到逻辑端的中性意图词汇，领域意图经标签开放扩展 */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SingularisInteraction_Intent);

/** 意图：主张（交互者发起交互，会话建立的唯一触发） */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SingularisInteraction_Intent_Press);

/** 意图：撤回（交互者结束交互参与） */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SingularisInteraction_Intent_Release);

/** 意图：值（连续量更新，轴值 / 力度） */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SingularisInteraction_Intent_Value);

/** 事件域根标签：逻辑端到响应与表现的会话事件词汇，领域事件经标签开放扩展 */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SingularisInteraction_Event);

/** 事件：会话建立（离散事件） */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SingularisInteraction_Event_Begin);

/** 事件：会话推进（唯一连续事件） */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SingularisInteraction_Event_Progress);

/** 事件：会话定格（离散事件；如拖动松手后的中间态落定） */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SingularisInteraction_Event_Settle);

/** 事件：会话达成（离散事件） */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SingularisInteraction_Event_Complete);

/** 事件：会话中止（离散事件；含准入拒绝反馈） */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SingularisInteraction_Event_Cancel);

/** 响应域根标签 */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SingularisInteraction_Response);

/** 默认响应标签，绑定默认输入动作 */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SingularisInteraction_Response_Default);
