/**
 * @file      octets.h
 * @brief     Portable byte string implementation
 * @author    Junheng Zang (junheng.zang@gmail.com)
 * @copyright Copyright (c) 2012 Junheng Zang. All Rights Reserved.
 */
#ifndef ZBASE__OCTETS_H
#define ZBASE__OCTETS_H

#include <string>
#include <sstream>

#include <zbase/config.h>
#include <zbase/atomic.h>

/**
 * @namespace zbase
 */
namespace zbase
{
	/**
	 * @class Octets
	 * @brief Byte string container
	 */
	class Octets
	{
	public:
		/**
		 * @brief Invalid position
		 */
		static const size_t npos = static_cast<size_t>(-1);

	private:
		/**
		 * @class Rep
		 * @brief Internal representation of Octets
		 * @details ATTENTION: 
		 *   1. Modify existing data content is not allowed. clone() shoule be used to create a new object in this case.
		 *      This is how copy-on-write(COW) works.
		 *   2. All method parameters must be checked outside before invoking any method
		 *   3. Proxy design pattern is used to implement Copy-On-Write strategy. (Octets is a virtual proxy of Rep)
		 */
		class Rep
		{
		public:
			/**
			 * @brief Constructor
			 */
			Rep(const void* data, size_t size);
			/**
			 * @brief Clone
			 */
			Rep* Clone() { return new Rep(m_data, m_size); }

			/**
			 * @brief Get internal data buffer
			 */
			const void* GetData() const { return m_data; }
			/**
			 * @brief Get internal data buffer
			 */
			void* GetData() { return m_data; }
			/**
			 * @brief Get size of the internal data buffer
			 */
			size_t GetCapacity() const { return m_capacity; }
			/**
			 * @brief Set data size
			 */
			void SetSize(size_t size) { m_size = size; }
			/**
			 * @brief Get data size
			 */
			size_t GetSize() const { return m_size; }
			/**
			 * @brief Check if the object memory is leaked
			 */
			bool IsLeaked() const { return m_refno < 0; }
			/**
			 * @brief Check if this object is shared
			 */
			bool IsShared() const { return m_refno > 0; }
			/**
			 * @brief Get string description in hex format
			 */
			std::string Hex();

			/**
			 * @brief Reserve the internal buffer to at least specified size
			 */
			void Reserve(size_t size);
			/**
			 * @brief Increase reference number
			 */
			void AddRef()
			{
#ifdef ZBASE_MULTITHREADS
				atomic::IncAndFetch(&m_refno);
#else
				++m_refno;
#endif
			}
			/**
			 * @brief Decrease reference number
			 * @details Deallocate memory if the reference number is less than 0
			 */
			void Release()
			{
#ifdef ZBASE_MULTITHREADS
				if (atomic::DecAndFetch(&m_refno) < 0) {
					delete this;
				}
#else
				if ((--m_refno) < 0) {
					delete this;
				}
#endif
			}

		// forbid using destructor, copy constructor and assignment operator externally
		private:
			/**
			 * @brief Destructor
			 * @details Forbid external use
			 */
			~Rep();
			/**
			 * @brief Copy constructor
			 * @details Forbid external use
			 */
			Rep(const Rep& r) {}
			/**
			 * @brief Operator =
			 * @details Forbid external use
			 */
			Rep& operator = (const Rep& rhs) { return *this; }

		private:
			/**
			 * @brief Internal data buffer
			 */
			void  *m_data;
			/**
			 * @brief Size of the internal data buffer
			 */
			size_t m_capacity;
			/**
			 * @brief Size of the actual data
			 */
			size_t m_size;
			/**
			 * @brief Reference number
			 */
			int    m_refno;
		};

	public:
		/**
		 * @brief Constructor
		 */
		Octets();
		/**
		 * @brief Constructor
		 */
		Octets(const void *data, size_t size);
		/**
		 * @brief Constructor
		 */
		Octets(const std::string& str);
		/**
		 * @brief Destructor
		 */
		~Octets();

		/**
		 * @brief Copy constructor
		 */
		Octets(const Octets& other);
		/**
		 * @brief Assignment operator
		 */
		Octets& operator = (const Octets& rhs);

		/**
		 * @adddtogroup Member accessors
		 * {@
		 */

		/**
		 * @brief Get intrnal data buffer
		 * @details ATTENTION: The following accessors are for readonly access. 
		 *          External change through these accessors will violate reference count of Rep and is forbidden.
		 */
		const void* GetData() const { return NULL == m_rep ? NULL : m_rep->GetData(); } 
		/**
		 * @brief Get data size
		 */
		size_t GetSize() const { return NULL == m_rep ? 0 : m_rep->GetSize(); }
		/**
		 * @brief Check if it is empty
		 */
		bool IsEmpty() const { return NULL == m_rep || m_rep->GetSize() == 0; }
		/**
		 * @brief Compare with another Octets object
		 * @param [in] rhs: Another Octets object to compare
		 * @return -1 for less; 0 for equal; 1 for greater
		 */
		int Compare(const Octets& rhs) const;

		/** @} */

		/**
		 * @addtogroup Member modifiers
		 * {@
		 */

		/**
		 * @brief Assign content
		 */
		Octets& Assign(const void *data, size_t size);
		/**
		 * @brief Append to the existing data
		 */
		Octets& Append(const void *data, size_t size);
		/**
		 * @brief Append to the existing data
		 */
		Octets& Append(const Octets& o);
		/**
		 * @brief Swap data of two Octets ojbects
		 */
		void Swap(Octets& rhs);
		/**
		 * @brief Clear the object to be empty
		 */
		void Clear();
		/**
		 * @brief Get string description in hex format
		 */
		std::string Hex() { return NULL == m_rep ? "" : m_rep->Hex(); }

		/** @} */

		/**
		 * @addtogroup Operators
		 * {@
		 */
		/** @brief Operator += */
		Octets& operator += (const Octets& rhs) { return Append(rhs.GetData(), rhs.GetSize()); }
		/** @brief Operator == */
		bool operator == (const Octets& rhs) { return GetSize() == rhs.GetSize() && Compare(rhs) == 0; }
		/** @brief Operator != */
		bool operator != (const Octets& rhs) { return GetSize() != rhs.GetSize() || Compare(rhs) != 0; }
		/** @brief Operator > */
		bool operator > (const Octets& rhs) { return Compare(rhs) > 0; }
		/** @brief Operator >= */
		bool operator >= (const Octets& rhs) { return Compare(rhs) >= 0; }
		/** @brief Operator < */
		bool operator < (const Octets& rhs) { return Compare(rhs) < 0; }
		/** @brief Operator <= */
		bool operator <= (const Octets& rhs) { return Compare(rhs) <= 0; }

		/** @} */

	private:
		/**
		 * @brief Internal data holder
		 */
		Rep *m_rep;
	};

	/**
	 * @brief Operator +
	 */
	Octets operator + (const Octets& a, const Octets& b);

	/**
	 * @brief Output a Octets object to a ostringstream object
	 */
	std::ostringstream& operator << (std::ostringstream& oss, const Octets& data);
	/**
	 * @brief Output a Octets object to a ostream object
	 */
	std::ostream& operator << (std::ostream& oss, const Octets& data);

} // namespace zbase
#endif // ZBASE__OCTETS_H

