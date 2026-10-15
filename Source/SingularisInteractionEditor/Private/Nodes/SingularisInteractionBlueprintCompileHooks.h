/* ====================================================================== *
 * SingularisInteractionBlueprintCompileHooks.h                           *
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
#include <UObject/ObjectCompileContext.h>
#include <UObject/ObjectMacros.h>

class UBlueprint;
class UObject;
class FDelegateHandle;

/**
 * 蓝图编译期声明回填 hook（编辑器模块）。
 *
 * 订阅 FCoreUObjectDelegates::OnObjectPostCDOCompiled，在蓝图产物 CDO 编译完成后
 * 扫描事件图内的会话事件节点，把「事件标签 → 事件函数名」写入响应类 CDO 的
 * DeclaredSessionEventHandlers（事件节点由编译器生成独立 stub UFunction，可按名直调）。
 * 该字段为可烘焙 UPROPERTY：随蓝图资产序列化并参与烘焙，打包版本运行时无需编辑器即可读取与调用。
 */
class FSingularisInteractionBlueprintCompileHook
{
public:
	/**
	 * 注册 CDO 编译回调。
	 * @param OutHandle 输出的委托句柄，供 Unregister 解绑使用。
	 */
	static void Register(FDelegateHandle& OutHandle);

	/**
	 * 解绑 CDO 编译回调。
	 * @param Handle 待解绑的委托句柄，解绑后复位。
	 */
	static void Unregister(FDelegateHandle& Handle);

	/**
	 * CDO 编译完成回调：扫描图内声明并回填响应类 CDO。
	 * @param CDO 刚编译完成的类默认对象（骨架类编译被跳过）。
	 * @param Context 编译上下文（含骨架编译标志）。
	 */
	static void HandleCDOCompiled(UObject* CDO, const FObjectPostCDOCompiledContext& Context);
};
