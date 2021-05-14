#ifndef BTREE_ITERATOR_H
#define BTREE_ITERATOR_H

#include <iterator>
#include <memory>

template <typename T> class btree;
template <typename T> class const_btree_iterator;

template <typename T> 
class btree_iterator {
public:
        typedef T value_type;
        typedef T* pointer;
        typedef T& reference;

	typedef std::ptrdiff_t difference_type;
	typedef std::forward_iterator_tag iterator_category;

	typedef typename btree<T>::Node Node;
	typedef const_btree_iterator<T> const_btree_iter;

	/** Constructor
	*/
	btree_iterator() = default;
	btree_iterator(Node* n, unsigned int p = 0U, bool isValid = true) 
		: currPtr(n), pos(p), isValid(n == nullptr ? false:isValid)
	{}
	btree_iterator(const btree_iterator& b) = default;
	btree_iterator(btree_iterator&& b) noexcept = default;

	reference operator*() const {
		return currPtr->elems[pos];
	}
	pointer operator->() const {
		return &(operator*());
	}

	btree_iterator& operator++() {
		if(currPtr == nullptr)
			return *this;
    	if(!isValid){
    		isValid=true;
    		return *this;
    	}
    	auto prevValue = currPtr->elems[pos];
    	while(currPtr->elems[pos]<=prevValue && isValid){
    		if(pos < currPtr->elems.size() - 1) {
    			++pos;
    			while(currPtr->hasChildAt(pos)){
    				currPtr = currPtr->getLeftChildAt(pos);
    				pos = 0;
    			}
    		}
    		else {
    			if(pos == currPtr->elems.size()-1 && currPtr->hasRightChild()){
    				currPtr = currPtr->getRightChild();
    				pos = 0;
    			}
    			else {
					if (currPtr->hasParent()) {
	                    auto oldPos = pos;
	                    auto oldPtr = currPtr;
	                    while(prevValue >= currPtr->elems[pos] &&  pos == currPtr->elems.size()-1) {
	                        if (!currPtr->hasParent()) {
	                            pos = oldPos;
	                            currPtr = oldPtr;
	                            isValid = false;
	                            break;
	                        }
	                        pos = currPtr->posInParent;
	                        currPtr = currPtr->parent;
	                    }

	                }
	                else {
	                    isValid = false;
	                    break;
	                }
    			}
    		}
    	}
    	return *this;
    }


    btree_iterator& operator--(){
	    if (currPtr == nullptr) {
	        return *this;
	    }
	    if (!isValid) {
	        isValid = true;
	        return *this;
	    }
	    auto prevValue = currPtr->elems[pos];

	    while (currPtr->elems[pos] >= prevValue && isValid) {
	        if (pos > 0) {
	            if (currPtr->hasChildAt(pos)) {
	                currPtr = currPtr->getLeftChildAt(pos);
	                pos = currPtr->elems.size()-1;
	                while(currPtr->hasRightChild()) {
	                    currPtr = currPtr->getRightChild();
	                    pos = currPtr->elems.size()-1;
	                }
	            } else {
	                --pos;
	            }
	        } else {
	            if (currPtr->hasChildAt(pos)) {
	                while(currPtr->hasChildAt(pos)) {
	                    currPtr = currPtr->getLeftChildAt(pos);
	                    pos = currPtr->elems.size()-1;
	                }
	            } else {
	                if (currPtr->hasParent()) {
	                    auto oldPos = pos;
	                    auto oldPtr = currPtr;

	                    while(currPtr->hasParent()) {
	                        pos = currPtr->posInParent;
	                        currPtr = currPtr->parent;
	                        if (pos > 0) {
	                            break;
	                        }
	                    }
	                    if (pos == 0) {
	                        pos = oldPos;
	                        currPtr = oldPtr;
	                        isValid = false;
	                    } else if (currPtr->elems[pos] > prevValue) {
	                        --pos;
	                    }
	                }
	            }
	        }
	    }
	    return *this;
	}

    btree_iterator operator++(int) {
    	auto tmp = *this;
    	++*this;
    	return tmp;
    }

    btree_iterator operator--(int){
    	auto tmp = *this;
    	--*this;
    	return tmp;
    }

    bool operator==(const btree_iterator& other) const {
    	if (isValid == false && other.isValid == false)
        	return true;
    	return currPtr == other.currPtr && pos == other.pos && isValid == other.isValid;
    }
    bool operator!=(const btree_iterator& other) const{
    	return !operator==(other);
    }
    
    bool operator==(const const_btree_iter& other) const {
        return other == *this;
    }
    bool operator!=(const const_btree_iter& other) const {
        return other != *this;
    }
    

private:
	Node* currPtr {nullptr};
	unsigned int pos {0U};
	bool isValid {false};

};


template <typename T> 
class const_btree_iterator {
public:
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;

    typedef std::ptrdiff_t difference_type;
    typedef std::forward_iterator_tag iterator_category;

    typedef typename btree<T>::Node Node;
    typedef btree_iterator<T> btree_iter;

    const_btree_iterator() = default;
    const_btree_iterator(Node* n, unsigned int p) : btree_it{n, p} {}

    const_btree_iterator(const btree_iter& b) : btree_it{b} {}
    const_btree_iterator(const const_btree_iterator& b) = default;
    const_btree_iterator(const_btree_iterator&& b) noexcept = default;

    reference operator*() const {
        return *btree_it;
    }
    pointer operator->() const {
        return &(operator*());
    }
    const_btree_iterator& operator++() {
        ++btree_it;
        return *this;
    }
    const_btree_iterator& operator--() {
        --btree_it;
        return *this;
    }
    
    bool operator==(const btree_iter& other) const {
        return btree_it == other;
    }
    bool operator!=(const btree_iter& other) const {
        return btree_it != other;
    }

    bool operator==(const const_btree_iterator& other) const {
        return btree_it == other.btree_it;
    }
    bool operator!=(const const_btree_iterator& other) const {
        return !operator==(other);
    }

private:
    btree_iter btree_it {};
};

#endif

