#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <cstddef>
#include <utility>
#include <iterator>
#include <queue>
#include <vector>
#include <memory>

#include "btree_iterator.h"

const size_t DEFAULT_MAX_NODE_SIZE = 40;


template <typename T> std::ostream& operator<<(std::ostream& os, const btree<T>& tree);

template <typename T> 
class btree {
public:
	friend class btree_iterator<T>;
	friend class const_btree_iterator<T>;

	typedef btree_iterator<T> iterator;
	typedef const_btree_iterator<T> const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	/** Constructor
	*/
	btree() = default;
	btree(size_t t)
		: maxNodeSize(t)
	{};
	btree(const btree<T>& other)
		: maxNodeSize(other.maxNodeSize)
	{
		auto q = std::queue<Node*>{};
		q.push(other.root.get());

		while(! q.empty()) {
			auto curr = q.front();
			q.pop();
			for(unsigned int i = 0; i < curr->elems.size(); ++i) {
				insert(curr->elems[i]);
				if (curr->hasChildAt(i))
					q.push(curr->getLeftChildAt(i));
			}
			if (curr->hasRightChild())
				q.push(curr->getRightChild());
		}
	}

	btree(btree<T>&& other) noexcept = default;
	~btree() = default;


	/** Operator
	*/
	btree<T>& operator=(const btree<T>& other){
		btree<T>{other}.swap(*this);
		return *this;
	}
	btree<T>& operator=(btree<T>&& other) noexcept = default;

	
	/** Iterator
	*/
	iterator begin() const {
		if (root == nullptr || root->isEmpty())
			return iterator(end());
		auto curr = root.get();
		while(curr->hasChildAt(0)){
			curr = curr->getLeftChild();
		}
		return iterator(curr);
	}
	iterator end() const {
		return iterator(maxElement());
	}
	const_iterator cbegin() const {
		return const_iterator(begin());
	}
	const_iterator cend() const {
		return const_iterator(end());
	}
	reverse_iterator rbegin() const {
		return reverse_iterator(end());
	}
	reverse_iterator rend() const{
		return reverse_iterator(begin());
	}
	const_reverse_iterator crbegin() const {
		return reverse_iterator(end());	
	}
	const_reverse_iterator crend() const {
		return reverse_iterator(begin());
	}

	/** Methods
	*/
	std::pair<iterator, bool> insert(const T& elem){
		// empty tree
		if(root == nullptr || root->isEmpty()) {
			root = std::make_unique<Node>(0,nullptr, elem, maxNodeSize);
			return std::make_pair(iterator(root.get(), 0), true);
		}
		auto curr = root.get();
		unsigned int i = 0;
		// full tree
		while(curr->isFull()){
			if(elem < curr->elems[i]){
				if (curr->getLeftChildAt(i) == nullptr)
					curr->createLeftChildAt(i);
				curr = curr->getLeftChildAt(i);
				i=0;
			}
			else if (elem > curr->elems[i]){
				if(i < maxNodeSize-1)
					++i;
				else{
					if(curr->getRightChild()==nullptr)
						curr->createRightChild();
					curr = curr->getRightChild();
					i = 0;
				}
			}
			else
				return std::make_pair(iterator(curr, i), false);
		}
		// insert elem in sorted position
		auto arr = curr->elems;
		while (i < arr.size()) {
			if (elem == arr[i])
				return std::make_pair(iterator(curr, i), false); 
			else if (elem < arr[i])
				break;
			else
				++i;
    		}
		curr->emplaceElementBefore(i, elem);
		return std::make_pair(iterator(curr, i), true);
	}

	friend std::ostream& operator<< <T>(std::ostream& out, const btree<T>& b);

	iterator find(const T& elem){
		return findElem(elem);
	}
	const_iterator find(const T& elem) const {
		return findElem(elem);
	}

	iterator maxElement() const {
		auto curr = root.get();
		while (curr->hasRightChild())
			curr = curr->getRightChild();
		return iterator(curr, curr->elems.size()-1, false);
	}
	void clear() noexcept {
		maxNodeSize = DEFAULT_MAX_NODE_SIZE;
		root.reset(nullptr);
	}
	void swap(btree<T>& other) noexcept {
		std::swap(maxNodeSize, other.maxNodeSize);
		std::swap(root, other.root);
	}
	iterator findElem(const T& elem) const {
		if(root==nullptr || root->isEmpty())
			return end();
		auto curr = root.get();
		unsigned int i=0;
		while(curr->isFull()){
			if(elem < curr->elems[i]){
				if(curr -> hasChildAt(i)){
					curr = curr->getLeftChildAt(i);
					i=0;
				}
				else 
					return end();
			}
			else if (elem > curr->elems[i]){
				if(i<maxNodeSize-1)
					++i;
				else {
					if(curr->hasRightChild()){
						curr = curr->getRightChild();
						i=0;
					}
					else
						return end();
				}
			}
			else{
				return iterator(curr, i);
			}
		}
		auto elems = curr->elems;
		while (i < elems.size()) {
			if (elem == elems[i]) {
				return iterator(curr, i);
			} else if (elem < elems[i]) {
				break;
			} else {
				++i;
			}
		}
		return end();
	}

	/** Node
	*/
	struct Node {
		// Constructor
		Node() 
		{
			clearChildren();
		}
		Node(unsigned int posP, Node* p, const size_t m) 
			: posInParent(posP), parent(p), maxNodeSize(m)
		{
			clearChildren();
		}
		Node(unsigned int posP, Node* p, const T& val, const size_t m)
			: posInParent(posP), parent(p), maxNodeSize(m)
		{
			clearChildren();
			elems.emplace(elems.cbegin(), val);
		}
		~Node() = default;
		// Methods
		    bool hasParent() const {
        		return parent!=nullptr;
        	}
        	void createLeftChildAt(unsigned int i) {
        		leftChildren[i] = std::make_unique<Node>(i,this,maxNodeSize);
       		}
        	void createRightChild(){
        		rightChild = std::make_unique<Node>(elems.size()-1,this,maxNodeSize);
        	}
        	Node* getLeftChildAt(unsigned int i) const{
        		return leftChildren[i].get();
        	}
        	Node* getRightChild() const{
        		return rightChild.get();
        	}
        	Node* getLeftChild() const{
        		return getLeftChildAt(0);
        	}
        	bool hasChildAt(unsigned int i) const {
        		return getLeftChildAt(i)!=nullptr;
        	}
        	bool hasRightChild() const {
        		return rightChild!=nullptr;
        	}
     	   void clearChildren(){
				leftChildren.clear();
				for(auto i=0U; i<maxNodeSize;i++)
					leftChildren.push_back(nullptr);
				rightChild = nullptr;
			}
        	bool containsElement(const T& v) const {
        		return std::find(elems.cbegin(), elems.cend(), v);
        	}
        	bool isFull() const {
        		return elems.size() == maxNodeSize;
        	}
        	bool isEmpty() const {
        		return elems.empty();
        	}
        	void emplaceElementBefore(unsigned int index, const T& elem){
        		if(!isFull())
        			elems.emplace(elems.cbegin()+index, elem);
		}
		// Node elements
		unsigned int posInParent {0};
		Node* parent {nullptr};
		size_t maxNodeSize {DEFAULT_MAX_NODE_SIZE};
		std::vector<T> elems {};
		std::vector<std::unique_ptr<Node>> leftChildren {};
		std::unique_ptr<Node> rightChild {nullptr};
	};

	size_t maxNodeSize {DEFAULT_MAX_NODE_SIZE};
	std::unique_ptr<Node> root {nullptr};
};

template<typename T>
std::ostream& operator<<(std::ostream& out, const btree<T>& b){
	if(b.root == nullptr){
		auto q = std::queue<typename btree<T>::Node*>{};
		q.push(b.root.get());
		while(!q.empty()){
			auto curr = q.front();
			q.pop();
			for(unsigned int i = 0; i<curr->elems.size();++i){
				if(i>0 || curr != b.root.get())
					out <<  ' ';
				out << curr ->elems[i];
				if(curr->hasChildAt(i))
					q.push(curr->getLeftChildAt(i));
			}
			if(curr->hasRightChild())
				q.push(curr->getRightChild());
		}
	}
	return out;
}

#endif
