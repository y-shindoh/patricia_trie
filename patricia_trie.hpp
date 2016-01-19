/* -*- coding: utf-8; tab-width: 4 -*- */
/**
 * @file	patricia_trie.hpp
 * @brief	パトリシア木
 * @author	Yasutaka SHINDOH / 新堂 安孝
 */

#ifndef	__PATRICIA_TRIE_HPP__
#define	__PATRICIA_TRIE_HPP__	"patricia_trie.hpp"

#include <cstring>
#include <cassert>
#include <unordered_map>
#include <algorithm>

namespace ys
{
	/**
	 * パトリシア木
	 * @note	テンプレートのパラメータ @a VTYPE には符号なし整数を与えること。
	 */
	template<typename KTYPE, typename VTYPE>
	class PatriciaTrie
	{
	private:

		/**
		 * パトリシア木の内部で用いるノード
		 */
		template<typename K_, typename N_>
		class Node
		{
		public:

			K_* d_;	///< キーの全体
			N_ l_;	///< キーの全体の長さ
			N_ v_;	///< キー末端
			std::unordered_map<K_, Node<K_, N_>*> c_;	///< 子ノード

			/// フィールド @a v_ の不正値
			static const N_ INV_ = ~(N_)0;

		private:

			/**
			 * コンストラクタ
			 * @param[in]	key	キー
			 * @param[in]	length	配列 @a key の要素数
			 * @param[in]	value	キーに対応した値
			 * @note	引数 @a value が @a INV_ の時は、ノードは非末端。
			 */
			Node(const K_* key,
				 N_ length,
				 N_ value)
				: l_(length), v_(value)
				{
					assert(key);
					assert(0 < length);

					d_ = new K_[length];

					std::memcpy((void*)d_, (const void*)key, sizeof(K_) * length);
				}

			/**
			 * キーの全体から先頭を除去
			 * @param[in]	length	除去する要素数
			 */
			void
			cut_head(N_ length)
				{
					assert(length < l_);

					l_ -= length;
					K_* d = new K_[l_];

					std::memcpy((void*)d, (const void*)(d_ + length), sizeof(K_) * l_);

					delete [] d_;
					d_ = d;
				}

		public:

			/**
			 * デストラクタ
			 */
			~Node()
				{
					delete [] d_;

					for (auto v : c_) { delete v.second; }
				}

			/**
			 * キーを削除
			 * @param[in]	key	削除対象のキー
			 * @param[in]	length	配列 @a key の長さ
			 * @return	キー @a key に対応した値
			 * @todo	必要であればエッジの統合を実現する。
			 */
			N_
			remove_key(const K_* key,
					   N_ length)
				{
					assert(key);
					assert(0 < length);
					assert(d_[0] == key[0]);

					if (length < l_) return INV_;

					for (N_ i(1); i < l_; ++i) {
						if (d_[i] != key[i]) return INV_;
					}
					if (length == l_) {
						N_ r(v_);
						v_ = INV_;
						return r;
					}

					auto it = c_.find(key[l_]);
					if (it == c_.end()) return INV_;

					return it->second->remove_key(key + l_, length - l_);
				}

			/**
			 * ノードを探索
			 * @param[in]	key	探索対象のキー
			 * @param[in]	length	配列 @a key の長さ
			 * @return	キーに対応した値
			 * @note	キーが見つからなかったときは @a INV_ を返す。
			 */
			N_
			get_value(const K_* key,
					  N_ length) const
				{
					assert(key);
					assert(0 < length);
					assert(d_[0] == key[0]);

					if (length < l_) return INV_;

					for (N_ i(1); i < l_; ++i) {
						if (d_[i] != key[i]) return INV_;
					}
					if (length == l_) return v_;

					auto it = c_.find(key[l_]);
					if (it == c_.end()) return INV_;

					return it->second->get_value(key + l_, length - l_);
				}

			/**
			 * ノードを探索 (共通接頭辞探索)
			 * @param[in]	buffer	探索対象のデータ
			 * @param[in]	length	配列 @a buffer の長さ
			 * @param[out]	values	配列 @a buffer の接頭辞となるキーの全ての値
			 */
			void
			get_values(const K_* buffer,
					   N_ length,
					   std::vector<N_>& values) const
				{
					assert(buffer);
					assert(0 < length);
					assert(d_[0] == buffer[0]);

					if (length < l_) return;
					if (v_ != INV_) values.push_back(v_);

					if (l_ < length) {
						auto it = c_.find(buffer[l_]);
						if (it != c_.end()) {
							it->second->get_values(buffer + l_, length - l_, values);
						}
					}
				}

			/**
			 * ノードにキーを追加
			 * @param[in,out]	node	追加対象のノード
			 * @param[in]	key	キー
			 * @param[in]	length	配列 @a key の要素数
			 * @param[in]	value	キーに対応した値
			 * @note	引数 @a value には @a INV_ を代入してはいけない。
			 * @return	追加後のノード
			 */
			static Node<K_, N_>*
			Add(Node<K_, N_>* node,
				const K_* key,
				N_ length,
				N_ value = 0)
				{
					assert(key);
					assert(0 < length);
					assert(value != INV_);	// 非末端ノードは生成しない

					if (node) {
						assert(node->d_[0] == key[0]);

						N_ i(1);
						N_ n = std::min(node->l_, length);

						while (i < n) {
							if (node->d_[i] != key[i]) break;
							++i;
						}

						if (i < n) {
							// 分離
							Node<K_, N_>* p = new Node<K_, N_>(key, i, INV_);
							p->c_[node->d_[i]] = node;
							node->cut_head(i);
							p->c_[key[i]] = new Node<K_, N_>(key + i, length - i, value);
							node = p;
						}
						else {
							if (node->l_ < length) {
								// 追加
								Node<K_, N_>* c(0);
								if (node->c_.find(key[i]) != node->c_.end()) c = node->c_[key[i]];
								node->c_[key[i]] = Node<K_, N_>::Add(c, key + i, length - i, value);
							}
							else if (length < node->l_) {
								// 追加
								Node<K_, N_>* p = new Node<K_, N_>(key, length, value);
								p->c_[node->d_[i]] = node;
								node->cut_head(i);
								node = p;
							}
							else {
								// 更新
								node->v_ = value;
							}
						}
					}
					else {
						node = new Node<K_, N_>(key, length, value);
					}

					return node;
				}
		};

		std::unordered_map<KTYPE, Node<KTYPE, VTYPE>*> head_;	///< ノード群

	public:

		/// キーに対応する値の不正値
		static const VTYPE INVALID_ = Node<KTYPE, VTYPE>::INV_;

		/**
		 * コンストラクタ
		 */
		PatriciaTrie() = default;

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
		 * @note	引数 @a value に @a INVALID_ を代入しないこと。
		 */
		void
		add_key(const KTYPE* key,
				VTYPE length,
				VTYPE value = 0)
			{
				assert(key);
				assert(0 < length);
				assert(value != INVALID_);

				Node<KTYPE, VTYPE>* node(0);
				if (head_.find(key[0]) != head_.end()) node = head_[key[0]];
				head_[key[0]] = Node<KTYPE, VTYPE>::Add(node, key, length, value);
			}

		/**
		 * キーを削除
		 * @param[in]	key	キー
		 * @param[in]	length	配列 @a key の要素数
		 * @return	キー @a key に対応する値
		 * @note	キーが見つからなかった場合は @a INVALID_ が返却される。
		 */
		VTYPE
		remove_key(const KTYPE* key,
				   VTYPE length)
			{
				assert(key);
				assert(0 < length);

				auto it = head_.find(key[0]);
				if (it == head_.end()) return false;
				return it->second->remove_key(key, length);
			}

		/**
		 * キーを探索 (キーに対応する値を獲得)
		 * @param[in]	key	キー
		 * @param[in]	length	配列 @a key の要素数
		 * @return	キー @a key に対応する値
		 * @note	キーが見つからなかった場合は @a INVALID_ が返却される。
		 */
		VTYPE
		get_value(const KTYPE* key,
				  VTYPE length) const
			{
				assert(key);
				assert(0 < length);

				auto it = head_.find(key[0]);
				if (it == head_.end()) return false;
				return it->second->get_value(key, length);
			}

		/**
		 * キーを探索 (共通接頭辞探索, 接頭辞となるキーの値を全て獲得)
		 * @param[in]	buffer	探索対象のデータ
		 * @param[in]	length	配列 @a buffer の要素数
		 * @param[out]	values	配列 @a buffer の接頭辞となるキーの全ての値
		 */
		void
		get_values(const KTYPE* buffer,
				   VTYPE length,
				   std::vector<VTYPE>& values) const
			{
				assert(buffer);
				assert(0 < length);

				auto it = head_.find(buffer[0]);
				if (it == head_.end()) return;
				it->second->get_values(buffer, length, values);
			}

		/**
		 * キーを探索 (キーの有無をチェック)
		 * @param[in]	key	キー
		 * @param[in]	length	配列 @a key の要素数
		 * @return	true: キーが見つかった, false: 見つからなかった
		 */
		bool
		find_key(const KTYPE* key,
				 VTYPE length) const
			{
				assert(key);
				assert(0 < length);

				return get_value(key, length) != INVALID_;
			}
	};
};

#endif	// __PATRICIA_TRIE_HPP__
