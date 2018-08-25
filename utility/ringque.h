/**
******************************************************************************
* @file    ringque.h
* @author  Chenxx
* @version V1.0
* @date    2016-9-19
* @brief   This file defines the ring queue.
******************************************************************************/
//release information:
//2016/10/21 chenxx: change front() and back() return type to reference
//2016/10/24 chenxx: fix pointer datatype bug in [uint16_t elemsInQue() const]
//2016/10/30 chenxx: add function empty() clear() capacity() need test.
//2016/11/04 chenxx: delete back(); add back_ptr(), front_ptr(); delete size()
//2017/01/14 chenxx: add call_back_push. Fix bug at align_que2array().
//2017/06/04 chenxx: add fixed member array. Expressions as follow are allowed:
//										ringque<char> buff(&array, 10);
//										ringque<char, 10> buff;

#ifndef RINGQUE_H
#define RINGQUE_H
#include <stdint.h>
#include <string.h>
template <typename T>
class ringque_base
{

public:

/**
	* @brief  	Constructor
	* @array:	static array for queue
	* @size: 	size of static array
	* @retval 	invalid
	*/
	ringque_base(T* array, uint16_t size):
		MEM_LEN_(size),
		static_array_(array),
		back_ptr_(static_array_),
		front_ptr_(static_array_)
	{
		if (array == NULL) while (1);
		memset(static_array_, 0, MEM_LEN_);
	}
	
	uint16_t capacity(){return (MEM_LEN_ - 1);}

	bool empty(){return (back_ptr_ == front_ptr_);}
	
	void clear(){back_ptr_ = front_ptr_;}

	T& front() {return *front_ptr_;}

/**
* @brief  return front pointer that can be used for memcpy
* @param  None
* @retval Pointer
*/
	T* front_ptr()
	{
		if (front_ptr_ > back_ptr_)
		{
			align_que2array();
		}
		return front_ptr_;
	}

/**
* @brief  return back pointer that can be used for memcpy
* @param  None
* @retval Pointer
*/
	T* back_ptr()
	{
		if(static_array_ == front_ptr_ || front_ptr_ > back_ptr_)
			return back_ptr_;
		
		align_que2array();
		return back_ptr_;
	}
/**
* @brief  push an element into queue
* @param  value
* @retval Success or Not
*/
	bool push(const T& value)
	{
		if (emptyElemsInQue() == 0) return 0;
		*back_ptr_ = value;
		if(++back_ptr_ >= static_array_ + MEM_LEN_) back_ptr_-= MEM_LEN_;
		return 1;
	}

/**
* @brief  pop an element from queue
* @param  None
* @retval return 0 if queue is empty
*/
	bool pop()
	{
		if (elemsInQue() == 0) return 0;
		if (++front_ptr_ >= static_array_ + MEM_LEN_) front_ptr_ -= MEM_LEN_;
		return 1;
	}

/**
* @brief  get elements number in queue
* @param  None
* @retval number of elements
*/
	uint16_t elemsInQue() const
	{
		int32_t elemsInQue = int32_t(back_ptr_ - front_ptr_);
		if (elemsInQue < 0) elemsInQue += MEM_LEN_;

		return (uint16_t)elemsInQue;
	}

/**
* @brief  get empty elements number in queue
* @param  None
* @retval number of empty elements
*/
	uint16_t emptyElemsInQue() const
	{
		return (MEM_LEN_ - 1 - elemsInQue());
	}

/**
* @brief  push array into queue
* @param  buf: pointer of the array
* @param  len: length of the array
* @retval the length actually pushed
*/
	uint16_t push_array(T* buf, uint16_t len)
	{
		uint16_t emptyElems_inQue = emptyElemsInQue();

		/* check if queue is full before push back */
		if (emptyElems_inQue == 0) return 0;

		/* check if queue overflow */
		if (len > emptyElems_inQue) 
			len = emptyElems_inQue;

		/* front pointer is before back pointer */
		if ((uint32_t)front_ptr_ <= (uint32_t)back_ptr_)
		{
			uint16_t bytes_arrayEnd_To_queueBack = MEM_LEN_ - ((uint32_t)back_ptr_ - (uint32_t)static_array_);//[1, TXBUF_SIZE]
			if (len >= bytes_arrayEnd_To_queueBack)
			{
				memcpy(back_ptr_, buf, bytes_arrayEnd_To_queueBack);
				memcpy(static_array_, buf + bytes_arrayEnd_To_queueBack, len - bytes_arrayEnd_To_queueBack);
				back_ptr_ = static_array_ + len - bytes_arrayEnd_To_queueBack;
			}
			else
			{
				memcpy(back_ptr_, buf, len);
				back_ptr_ += len;
			}
		}
		/* back pointer is before front pointer */
		else
		{
			memcpy(back_ptr_, buf, len);
			back_ptr_ += len;
		}
		return len;
	}

/**
* @brief  pop array from queue
* @param  buf: pointer of the array
* @param  len: length of the array
* @retval the length actually pushed
*/
	uint16_t pop_array(T* buf, uint16_t len)
	{
		uint16_t elems_inQue = elemsInQue();

		/* check if queue is empty before pop front */
		if (elems_inQue == 0) return 0;

		/* check if queue under flow */
		if (len > elems_inQue) len = elems_inQue;

		/* front pointer is before back pointer */
		if ((uint32_t)front_ptr_ <= (uint32_t)back_ptr_)
		{
			memcpy(buf, front_ptr_, len);
			front_ptr_ += len;
		}
		/* back pointer is before front pointer */
		else 
		{
			uint16_t bytes_arrayEnd_to_queueFront = MEM_LEN_ - ((uint32_t)front_ptr_ - (uint32_t)static_array_);//[1, TXBUF_SIZE]
			/* front pointer needs to jump back */
			if (len > bytes_arrayEnd_to_queueFront)
			{
				memcpy(buf, front_ptr_, bytes_arrayEnd_to_queueFront);
				memcpy(buf + bytes_arrayEnd_to_queueFront, static_array_, len - bytes_arrayEnd_to_queueFront);
				front_ptr_ = static_array_ + len - bytes_arrayEnd_to_queueFront;
			}
			else /* do not need to jump back */
			{
				memcpy(buf, front_ptr_, len);
				front_ptr_ += len;
			}
		}
		return len;
	}
	
	uint16_t call_back_push(uint16_t(*callback_fun)(void*, T*, uint16_t), void* obj, uint16_t len )
	{
		uint16_t free_size = emptyElemsInQue();
		len = len < free_size ? len : free_size;
		if(len == 0) return 0;
		len = callback_fun(obj, back_ptr(), len);
		back_ptr_ += len;
		return len;
	}
	private:

/**
* @brief  align the front pointer with the pointer of static array
* @param  None
* @retval None
*/
	void align_que2array()
	{
		uint16_t elems_inQue = elemsInQue();

		/* check if queue is empty before pop front */
		if (elems_inQue == 0) 
		{
			front_ptr_ = back_ptr_ = static_array_;
			return ;
		}
		
		/* check if queue is already aligned */
		if (static_array_ == front_ptr_) return;

		if(front_ptr_ > back_ptr_)
		{
			reverse(static_array_, front_ptr_ - 1);
			reverse(front_ptr_, static_array_ + MEM_LEN_ - 1);
			reverse(static_array_, static_array_ + MEM_LEN_ - 1);
		}else //front_ptr < back_ptr
		{
			memmove(static_array_, front_ptr_, elems_inQue);
		}
		
		front_ptr_ = static_array_;
		back_ptr_ = static_array_ + elems_inQue;
	}

/**
* @brief  reverse the pointer between first and last pointer
* @param  first_ptr 
* @param  last_ptr 
* @retval None
*/
	void reverse(T* first_ptr, T* last_ptr)
	{
		/* last_ptr must larger than first pointer */
		if (first_ptr > last_ptr)
		{
			T* temp_ptr = first_ptr;
			first_ptr = last_ptr;
			last_ptr = temp_ptr;
		}
		T tempValue;
		while(first_ptr < last_ptr)
		{
			tempValue = *first_ptr;
			*first_ptr = *last_ptr;
			*last_ptr = tempValue;
			first_ptr++;
			last_ptr--;
		}
	}
		const uint16_t MEM_LEN_;
		T* const static_array_;
		T* back_ptr_; //[front, back)
		T* front_ptr_;
};

template <typename T, uint16_t N = 0>
class ringque : public ringque_base<T>
{
public:
	
/**
	* @brief  	Constructor
	* @array:	static array for queue
	* @size: 	size of static array
	* @retval 	invalid
	*/
	ringque(T* array, uint16_t size):
		ringque_base<T>(array, size)
	{
	
	}
	
/**
	* @brief  	Constructor
	* @array:	static array for queue
	* @size: 	size of static array
	* @retval 	invalid
	*/
	ringque():
		ringque_base<T>(array_, N)
	{
		if(N == 0) while(1);
	}
	
private:
	T array_[N == 0 ? 1 : N];
};
#endif
//end of file
