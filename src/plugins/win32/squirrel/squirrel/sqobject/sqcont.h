/*
 * copyright (c)2009 http://wamsoft.jp
 * zlib license
 */
#ifndef __SQCONT_H__
#define __SQCONT_H__

/**
 * continuous handler 用処理
 * function(currentTick, diffTick) の形で定期的に呼び出されるメソッド群を登録する機能
 */
namespace sqobject {

/// 機能登録
void registerContinuous();
/// ハンドラ処理呼び出し。Thread::main の前で呼び出す必要がある
void beforeContinuous();
/// ハンドラ処理呼び出し。Thread::main の後で呼び出す必要がある
void afterContinuous();
/// 機能終了
void doneContinuous();

};

#endif
