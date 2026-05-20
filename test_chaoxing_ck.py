#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""学习通课表查询"""

import urllib.request
import json
import sys
import os

OUTPUT_FILE = os.path.join(os.path.expanduser('~'), 'Desktop', 'schedule_output.txt')

def fetch_schedule(cookie, week=None):
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36',
        'Cookie': cookie,
        'Referer': 'https://kb.chaoxing.com/'
    }
    url = 'https://kb.chaoxing.com/pc/curriculum/getMyLessons'
    if week:
        url += f'?week={week}'
    req = urllib.request.Request(url, headers=headers)
    with urllib.request.urlopen(req, timeout=15) as resp:
        return json.loads(resp.read().decode('utf-8'))

def begin_to_slot(begin):
    if begin <= 2: return 0
    elif begin <= 4: return 1
    elif begin <= 6: return 2
    elif begin <= 8: return 3
    else: return 4

def format_output(data, target_week):
    lessons = data['data']['lessonArray']
    schedule = {i: [''] * 5 for i in range(1, 8)}
    for lesson in lessons:
        weeks = lesson.get('weeks', '')
        week_list = [int(w) for w in weeks.split(',') if w.strip().isdigit()]
        if target_week not in week_list:
            continue
        day = lesson['dayOfWeek']
        slot = begin_to_slot(lesson['beginNumber'])
        schedule[day][slot] = lesson.get('name', '')
    weekdays = ['一', '二', '三', '四', '五', '六', '日']
    parts = ['221']
    for day in range(1, 8):
        parts.append(weekdays[day - 1] + ',' + ','.join(schedule[day]))
    return ';'.join(parts) + ';'

def main():
    print('请粘贴学习通Cookie:', end=' ', flush=True)
    cookie = input().strip()
    if not cookie:
        print('错误: Cookie不能为空')
        return

    data = fetch_schedule(cookie)
    current_week = data['data']['curriculum']['currentWeek']
    print(f'当前周次: {current_week}')

    target = input(f'查询第几周? (回车默认当前周{current_week}): ').strip()
    target_week = int(target) if target else current_week

    data2 = fetch_schedule(cookie, week=target_week)
    result = format_output(data2, target_week)
    print(f'\n=== 第{target_week}周 ===')
    print(result)

    with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
        f.write(result)
    print(f'\n结果已保存到桌面: {OUTPUT_FILE}')

if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print(f'错误: {e}', file=sys.stderr)
        try:
            with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
                f.write(f'ERROR: {e}')
        except Exception:
            pass
