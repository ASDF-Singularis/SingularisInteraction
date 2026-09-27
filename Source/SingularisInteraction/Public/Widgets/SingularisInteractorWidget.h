/* ====================================================================== *
 * SingularisInteractorWidget.h                                           *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/09/27 | Author: TrifingZW                                *
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
#include <Blueprint/UserWidget.h>

#include "Interfaces/SingularisInteractorViewInterface.h"
#include "SingularisInteractorWidget.generated.h"

class USingularisInteractionComponent;

/**
 * 引力奇点交互者控件。
 *
 * 默认交互者视图：实现 ISingularisInteractorViewInterface，框架（USingularisInteractorWidgetComponent）
 * 经接口推送交互者状态与事件。用户在蓝图或 C++ 子类中覆写 SPI，更新具体控件表现。
 */
UCLASS(Blueprintable)
class SINGULARISINTERACTION_API USingularisInteractorWidget : public UUserWidget,
                                                              public ISingularisInteractorViewInterface
{
	GENERATED_BODY()

public:
	/** 交互者状态全量刷新：当前锁定的交互目标。 */
	virtual void OnRefresh_Implementation(USingularisInteractionComponent* Target) override;

	/** 交互者锁定的目标变更。 */
	virtual void OnTargetChanged_Implementation(
		USingularisInteractionComponent* OldTarget,
		USingularisInteractionComponent* NewTarget
	) override;

	/** 交互者发起一次交互请求。 */
	virtual void OnTriggered_Implementation(
		USingularisInteractionComponent* Target,
		FGameplayTag StrategyTag
	) override;
};
