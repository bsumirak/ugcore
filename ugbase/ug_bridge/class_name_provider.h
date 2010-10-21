
#ifndef __H__UG_BRIDGE__CLASS_NAME_PROVIDER__
#define __H__UG_BRIDGE__CLASS_NAME_PROVIDER__

#include <vector>
#include <cstring>
#include <string>
#include <iostream>

namespace ug
{
namespace bridge
{

struct UG_ERROR_ClassUnknownToRegistry {};
struct UG_ERROR_ClassAlreadyNamed
{
		UG_ERROR_ClassAlreadyNamed(const char* name_)
			: name(name_)
		{}
		std::string name;
};

bool ClassNameVecContains(const std::vector<const char*>& names, const char* name);

template <typename TClass>
struct ClassNameProvider
{
	/// set name of class and copy parent names
		static void set_name(const char* nameIn, const char* group = "", bool newName = false)
		{
		//	if class already named throw error
			if(newName == true && !m_ownName.empty())
			{
				if(strcmp(nameIn, name()) != 0)
					throw(UG_ERROR_ClassAlreadyNamed(nameIn));
			}

		//	copy name into static string
		//	This is necessary, since char* could be to temporary memory
			m_ownName = std::string(nameIn);

		//	remember const char* to own name in first position of names-list
			m_names.clear();
			m_names.push_back(m_ownName.c_str());

		//	remember groups
			m_group = std::string(group);
		}

	/// set name of class and copy parent names
		template <typename TParent>
		static void set_name(const char* name, const char* group = "", bool newName = false)
		{
		//	set own name
			set_name(name, group, newName);

		//	copy parent names
			const std::vector<const char*>& pnames = ClassNameProvider<TParent>::names();

			for(size_t i = 0; i < pnames.size(); ++i)
				m_names.push_back(pnames[i]);
		}

		/** check if a given class name is equal
		 * This function checks if a given class name is equal to this class name.
		 * If the flag 'strict' is set to true, the class name must match exactly.
		 * If it is set to false, also parent names (of the class hierarchy) are checked
		 * and if this class inherits from the parent class true is returned.
		 */
		static bool is_a(const char* parent, bool strict = false)
		{
		//  strict comparison: must match this class name, parents are not allowed
			if(strict)
			{
			//  check pointer
				if(parent == name()) return true;

			//  compare strings
				if(strcmp(parent, name()) == 0) return true;

			//  names does not match
				return false;
			}

			return ClassNameVecContains(m_names, parent);
		}

		/// name of this class
		static const char* name()
		{
			if(m_ownName.empty())
				throw(UG_ERROR_ClassUnknownToRegistry());
			return m_ownName.c_str();
		}

		/// groups
		static const std::string& group()
		{
			return m_group;
		}

		/// returns vector of all names including parent class names
		static const std::vector<const char*>& names()	
		{
			if(m_names.empty())
				throw(UG_ERROR_ClassUnknownToRegistry());

			return m_names;
		}

	private:
		static std::string m_ownName;
		static std::vector<const char*> m_names;
		static std::string m_group;
};

template <typename TClass>
std::vector<const char*> ClassNameProvider<TClass>::m_names = std::vector<const char*>();

template <typename TClass>
std::string ClassNameProvider<TClass>::m_ownName = std::string("");

template <typename TClass>
std::string ClassNameProvider<TClass>::m_group = std::string("");

} // end namespace
} // end namespace

#endif /* __H__UG_BRIDGE__CLASS_NAME_PROVIDER__ */
