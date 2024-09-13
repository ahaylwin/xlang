#include "object.h"
#include "list.h"
#include "dict.h"
#include "xclass.h"
#include "attribute.h"

namespace X
{
	namespace Data
	{
		// Initialize the static atomic counter
		std::atomic<unsigned long long> Object::s_idCounter{ 0 };

		AttributeBag* Object::GetAttrBag()
		{
			AutoLock autoLock(m_lock);
			if (m_aBag == nullptr)
			{
				m_aBag = new AttributeBag();
			}
			return m_aBag;
		}
		void Object::DeleteAttrBag()
		{
			AutoLock autoLock(m_lock);
			if (m_aBag)
			{
				delete m_aBag;
				m_aBag = nullptr;
			}
		}
	}
}