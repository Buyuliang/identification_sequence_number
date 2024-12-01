#!/bin/bash

# 设置字符集（包含大小写字母和数字）
CHARSET="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

# 随机生成 9 个字符的函数
generate_random_string() {
  local str=""
  for i in {1..9}; do
    # 从字符集中随机选择一个字符并拼接
    str+="${CHARSET:RANDOM%${#CHARSET}:1}"
  done
  echo "$str"
}

# 无限循环，每隔 2 秒模拟输入一个 9 个字符的字符串
while true; do
  # 生成一个 9 个字符的随机字符串
  random_string=$(generate_random_string)
  # 使用 xdotool 输入字符串
  xdotool type "$random_string"
  # 每隔 2 秒输入一次
  sleep 2
done
