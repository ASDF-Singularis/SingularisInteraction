/* ====================================================================== *
 * SingularisInteractorWidgetFactory.cpp                                  *
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

#include "Factories/SingularisInteractorWidgetFactory.h"

#include <WidgetBlueprint.h>
#include <Kismet2/KismetEditorUtilities.h>
#include <Widgets/SingularisInteractorWidget.h>

USingularisInteractorWidgetFactory::USingularisInteractorWidgetFactory()
{
	// 关键配置
	bCreateNew = true; // 允许创建新资产
	bEditAfterNew = true; // 创建后自动打开编辑器（可选）
	SupportedClass = USingularisInteractorWidget::StaticClass(); // 关联具体的资产类
}

UObject* USingularisInteractorWidgetFactory::FactoryCreateNew(
	UClass* InClass,
	UObject* InParent,
	const FName InName,
	const EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn
)
{
	return FKismetEditorUtilities::CreateBlueprint(
		USingularisInteractorWidget::StaticClass(),
		InParent,
		InName,
		BPTYPE_Normal,
		UWidgetBlueprint::StaticClass(),
		// 必须指定为 UWidgetBlueprint
		UWidgetBlueprintGeneratedClass::StaticClass(),
		// 必须指定生成的类类型
		NAME_None
	);
}

bool USingularisInteractorWidgetFactory::ShouldShowInNewMenu() const
{
	return Super::ShouldShowInNewMenu();
}
