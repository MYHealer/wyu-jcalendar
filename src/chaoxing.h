#ifndef __CHAOXING_H__
#define __CHAOXING_H__

#include <Arduino.h>

// 从学习通获取本周课表，转换为J-Calendar格式并保存到Preferences
// 返回true表示成功
bool chaoxing_fetch_and_save();

#endif
