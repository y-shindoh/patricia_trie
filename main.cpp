/* -*- coding: utf-8; tab-width: 4 -*- */
/**
 * @file	main.cpp
 * @brief	patricia_trie.hppの動作確認用コマンド
 * @author	Yasutaka SHINDOH / 新堂 安孝
 */

#include <cstdio>
#include <cstring>
#include <vector>
#include "patricia_trie.hpp"

#define	N	8

/**
 * サンプル・コマンド
 */
int main()
{
	const char k[N][128] = {"これは日本語です。",
							"今日からがんばる。",
							"これは英語です。",
							"今日は雨です。",
							"今日からがんばる。",
							"ABCD.",
							"今日からがんばる。つもりです。",
							"これは"};

	ys::PatriciaTrie<char, unsigned int, unsigned int> pt;

	// キー登録 (偶数番目のみ)
	for (int i(0); i < N; ++i) {
		if (i % 2 != 0) continue;
		pt.add_key(k[i], std::strlen(k[i]), i);
	}

	pt.print();

	// キー探索
	for (int i(0); i < N; ++i) {
		unsigned int r = pt.get_value(k[i], std::strlen(k[i]));
		if (r != decltype(pt)::InvalidValue()) {
			std::printf("[%d: %u] %s\n", i, r, k[r]);
		}
		else {
			std::printf("[%d: -] %s\n", i, k[i]);
		}
	}

	const char b[] = "今日からがんばる。つもりです。うそです。";
	std::vector<unsigned int> v;

	// キー探索 (共通接頭辞)
	pt.get_values(b, std::strlen(b), v);
	for (auto i : v) {
		std::printf("[%u] %s\n", i, k[i]);
	}

	// キー探索 (キー削除+共通接頭辞)
	pt.remove_key(k[1], std::strlen(k[1]));
	v.clear();
	pt.get_values(b, std::strlen(b), v);
	for (auto i : v) {
		std::printf("[%u] %s\n", i, k[i]);
	}

	// キー探索 (キー追加+共通接頭辞)
	pt.add_key(k[1], std::strlen(k[1]), 1);
	v.clear();
	pt.get_values(b, std::strlen(b), v);
	for (auto i : v) {
		std::printf("[%u] %s\n", i, k[i]);
	}

	return 0;
}
