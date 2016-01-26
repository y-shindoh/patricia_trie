/* -*- coding: utf-8; tab-width: 4 -*- */
/**
 * @file	patricia_trie.hpp
 * @brief	C++ template library of patricia trie.
 * @author	Yasutaka SHINDOH / 新堂 安孝
 */

#ifndef	__PATRICIA_TRIE_HPP__
#define	__PATRICIA_TRIE_HPP__	"patricia_trie.hpp"

#if	!defined(__cplusplus) || __cplusplus < 201103L
#error	This library requires a C++11 compiler.
#endif

#include <cstdio>
#include <cstring>
#include <cassert>
#include <unordered_map>
#include <utility>

namespace ys
{
	/**
	 * パトリシア木
	 * @note	テンプレートのパラメータ @a LTYPE には、符号なし整数を与えること。
	 * @note	テンプレートのパラメータ @a INVALID には、@a VTYPE の不正値を与えること。
	 */
	template<typename KTYPE, typename LTYPE, typename VTYPE, VTYPE INVALID = ~(VTYPE)0>
	class PatriciaTrie
	{
	private:

		/**
		 * パトリシア木の内部で用いるノード
		 */
		template<typename K_, typename L_, typename V_, V_ I_ = INVALID>
		class Node
		{
		public:

			std::unordered_map<K_, Node<K_, L_, V_>*>* c_;	///< 子ノード
			K_* d_;	///< キーの全体 (0を許す)
			L_ l_;	///< キーの全体の長さ
			V_ v_;	///< キー末端

		private:

			/**
			 * コンストラクタ
			 * @param[in]	key	キー
			 * @param[in]	length	配列 @a key の要素数
			 * @param[in]	value	キーに対応した値
			 * @note	引数 @a value が @a I_ の時は、ノードは非末端。
			 */
			Node(const K_* key,
				 L_ length,
				 V_ value)
				: c_(0), d_(0), l_(length), v_(value)
				{
					assert(key);

					if (0 < length) {
						d_ = new K_[length];
						std::memcpy((void*)d_, (const void*)key, sizeof(K_) * length);
					}
				}

			/**
			 * キーの全体から先頭を除去
			 * @param[in]	length	除去する要素数
			 */
			void
			cut_head(L_ length)
				{
					assert(length <= l_);

					l_ -= length;
					if (d_) delete [] d_;

					if (0 < l_) {
						K_* d = new K_[l_];
						std::memcpy((void*)d, (const void*)(d_ + length), sizeof(K_) * l_);
						d_ = d;
					}
					else {
						d_ = 0;
					}
				}

		public:

			/**
			 * デストラクタ
			 */
			~Node()
				{
					if (d_) delete [] d_;

					if (c_) {
						for (auto c : *c_) { delete c.second; }
						delete c_;
					}
				}

			/**
			 * キーを削除
			 * @param[in]	key	削除対象のキー
			 * @param[in]	length	配列 @a key の長さ
			 * @return	キー @a key に対応した値
			 * @todo	必要であればエッジの統合を実現する。
			 */
			V_
			remove_key(const K_* key,
					   L_ length)
				{
					assert(key);

					if (length < l_) return I_;
					if (std::memcmp((const void*)d_, (const void*)key, sizeof(K_) * l_) != 0) return I_;
					if (length == l_) {
						V_ r(v_);
						v_ = I_;
						return r;
					}

					if (!c_) return I_;
					auto it = c_->find(key[l_]);
					if (it == c_->end()) return I_;

					return it->second->remove_key(key + (l_ + 1), length - (l_ + 1));
				}

			/**
			 * ノードを探索
			 * @param[in]	key	探索対象のキー
			 * @param[in]	length	配列 @a key の長さ
			 * @return	キーに対応した値
			 * @note	キーが見つからなかったときは @a I_ を返す。
			 */
			V_
			get_value(const K_* key,
					  L_ length) const
				{
					assert(key);

					if (length < l_) return I_;
					if (std::memcmp((const void*)d_, (const void*)key, sizeof(K_) * l_) != 0) return I_;
					if (length == l_) return v_;

					if (!c_) return I_;
					auto it = c_->find(key[l_]);
					if (it == c_->end()) return I_;

					return it->second->get_value(key + (l_ + 1), length - (l_ + 1));
				}

			/**
			 * ノードを探索 (共通接頭辞探索)
			 * @param[in]	buffer	探索対象のデータ
			 * @param[in]	length	配列 @a buffer の長さ
			 * @param[out]	values	配列 @a buffer の接頭辞となるキーの全ての値
			 */
			void
			get_values(const K_* buffer,
					   L_ length,
					   std::vector<V_>& values) const
				{
					assert(buffer);

					if (length < l_) return;
					if (std::memcmp((const void*)d_, (const void*)buffer, sizeof(K_) * l_) != 0) return;
					if (v_ != I_) values.push_back(v_);

					if (l_ < length && c_) {
						auto it = c_->find(buffer[l_]);
						if (it != c_->end()) {
							it->second->get_values(buffer + (l_ + 1), length - (l_ + 1), values);
						}
					}
				}

			/**
			 * ノードの状態を出力
			 * @param[in,out]	file	出力先
			 * @param[in]	d	ノードの深さ
			 * @note	出力形式は「<キーの先頭の値> +キーの長さ (キーに対応する値)」となる。
			 */
			void
			print(FILE* file,
				  const K_& k,
				  size_t d = 0) const
				{
					for (size_t i(0); i < d; ++i) std::fprintf(file, "  ");
					if (v_ == I_) {
						std::fprintf(file, "<%G> +%lu (-)\n", (double)k, (size_t)l_);
					}
					else {
						std::fprintf(file, "<%G> +%lu (%lu)\n", (double)k, (size_t)l_, (size_t)v_);
					}

					if (c_) {
						for (auto c : *c_) {
							c.second->print(file, c.first, d + 1);
						}
					}
				}

			/**
			 * ノードにキーを追加
			 * @param[in,out]	node	追加対象のノード
			 * @param[in]	key	キー
			 * @param[in]	length	配列 @a key の要素数
			 * @param[in]	value	キーに対応した値
			 * @note	引数 @a value には @a I_ を代入してはいけない。
			 * @return	追加後のノード
			 * @todo	メモリ確保失敗時の扱いについて考える。
			 */
			static Node<K_, L_, V_>*
			Add(Node<K_, L_, V_>* node,
				const K_* key,
				L_ length,
				V_ value = 0)
				{
					assert(key);
					assert(value != I_);	// 非末端ノードは生成しない

					if (node) {
						L_ i(0);
						L_ n = std::min(node->l_, length);

						while (i < n) {
							if (node->d_[i] != key[i]) break;
							++i;
						}

						if (i < n) {
							// 分離
							Node<K_, L_, V_>* p = new Node<K_, L_, V_>(key, i, I_);
							p->c_ = new std::unordered_map<K_, Node<K_, L_, V_>*>;
							(*p->c_)[node->d_[i]] = node;
							node->cut_head(i + 1);
							(*p->c_)[key[i]] = new Node<K_, L_, V_>(key + (i + 1), length - (i + 1), value);
							node = p;
						}
						else {
							if (node->l_ < length) {
								// 追加
								Node<K_, L_, V_>* c(0);
								if (node->c_) {
									if (node->c_->find(key[i]) != node->c_->end()) c = (*node->c_)[key[i]];
								}
								else {
									node->c_ = new std::unordered_map<K_, Node<K_, L_, V_>*>;
								}
								(*node->c_)[key[i]] = Node<K_, L_, V_>::Add(c, key + (i + 1), length - (i + 1), value);
							}
							else if (length < node->l_) {
								// 追加
								Node<K_, L_, V_>* p = new Node<K_, L_, V_>(key, length, value);
								p->c_ = new std::unordered_map<K_, Node<K_, L_, V_>*>;
								(*p->c_)[node->d_[i]] = node;
								node->cut_head(i + 1);
								node = p;
							}
							else {
								// 更新
								node->v_ = value;
							}
						}
					}
					else {
						node = new Node<K_, L_, V_>(key, length, value);
					}

					return node;
				}
		};

		std::unordered_map<KTYPE, Node<KTYPE, LTYPE, VTYPE>*> head_;	///< ノード群

	public:

		/**
		 * コンストラクタ
		 */
		PatriciaTrie() = default;

		/**
		 * コピー・コンストラクタ (使用禁止)
		 */
		PatriciaTrie(const PatriciaTrie<KTYPE, LTYPE, VTYPE, INVALID>&) = delete;

		/**
		 * 代入演算子 (使用禁止)
		 */
		PatriciaTrie&
		operator =(const PatriciaTrie<KTYPE, LTYPE, VTYPE, INVALID>&) = delete;

		/**
		 * デストラクタ
		 */
		virtual
		~PatriciaTrie()
			{
				for (auto n : head_) { delete n.second; }
			}

		/**
		 * キーを追加
		 * @param[in]	key	キー
		 * @param[in]	length	配列 @a key の要素数
		 * @param[in]	value	キー @a key に対応する値
		 * @note	引数 @a value に @a INVALID を代入しないこと。
		 */
		void
		add_key(const KTYPE* key,
				LTYPE length,
				VTYPE value = 0)
			{
				assert(key);
				assert(0 < length);
				assert(value != INVALID);

				Node<KTYPE, LTYPE, VTYPE>* node(0);
				if (head_.find(key[0]) != head_.end()) node = head_[key[0]];
				head_[key[0]] = Node<KTYPE, LTYPE, VTYPE>::Add(node, key + 1, length - 1, value);
			}

		/**
		 * キーを削除
		 * @param[in]	key	キー
		 * @param[in]	length	配列 @a key の要素数
		 * @return	キー @a key に対応する値
		 * @note	キーが見つからなかった場合は @a INVALID が返却される。
		 */
		VTYPE
		remove_key(const KTYPE* key,
				   LTYPE length)
			{
				assert(key);
				assert(0 < length);

				auto it = head_.find(key[0]);
				if (it == head_.end()) return false;
				return it->second->remove_key(key + 1, length - 1);
			}

		/**
		 * キーを探索 (キーに対応する値を獲得)
		 * @param[in]	key	キー
		 * @param[in]	length	配列 @a key の要素数
		 * @return	キー @a key に対応する値
		 * @note	キーが見つからなかった場合は @a INVALID が返却される。
		 */
		VTYPE
		get_value(const KTYPE* key,
				  LTYPE length) const
			{
				assert(key);
				assert(0 < length);

				auto it = head_.find(key[0]);
				if (it == head_.end()) return false;
				return it->second->get_value(key + 1, length - 1);
			}

		/**
		 * キーを探索 (共通接頭辞探索, 接頭辞となるキーの値を全て獲得)
		 * @param[in]	buffer	探索対象のデータ
		 * @param[in]	length	配列 @a buffer の要素数
		 * @param[out]	values	配列 @a buffer の接頭辞となるキーの全ての値
		 */
		void
		get_values(const KTYPE* buffer,
				   LTYPE length,
				   std::vector<VTYPE>& values) const
			{
				assert(buffer);
				assert(0 < length);

				auto it = head_.find(buffer[0]);
				if (it == head_.end()) return;
				it->second->get_values(buffer + 1, length - 1, values);
			}

		/**
		 * キーを探索 (キーの有無をチェック)
		 * @param[in]	key	キー
		 * @param[in]	length	配列 @a key の要素数
		 * @return	true: キーが見つかった, false: 見つからなかった
		 */
		bool
		find_key(const KTYPE* key,
				 LTYPE length) const
			{
				assert(key);
				assert(0 < length);

				return get_value(key, length) != INVALID;
			}

		/**
		 * キーの追加状態を出力
		 * @param[in,out]	file	出力先
		 * @note	出力形式はノード毎に「キーの長さ:キーの先頭の値 (キーに対応する値)」となる。
		 */
		void
		print(FILE* file = stdout) const
			{
				for (auto h : head_) h.second->print(file, h.first);
			}

		/**
		 * 値 @a INVALID を取得
		 * @return	値 @a INVALID
		 */
		static VTYPE
		InvalidValue()
			{
				return INVALID;
			}
	};
};

#endif	// __PATRICIA_TRIE_HPP__
