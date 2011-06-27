//	created by Sebastian Reiter
//	s.b.reiter@googlemail.com
//	y08 m11 d05

#ifndef __UTIL__ATTACHMENT_PIPE__
#define __UTIL__ATTACHMENT_PIPE__

#include <list>
#include <vector>
#include <stack>
#include <cassert>
#include "common/static_assert.h"
#include "common/types.h"
#include "common/util/uid.h"
#include "common/util/hash.h"
#include "page_container.h"

namespace ug
{

// PREDECLARATIONS
class IAttachment;
class IAttachmentDataContainer;


// CONSTANTS
enum ATTACHMENT_CONSTANTS
{
	INVALID_ATTACHMENT_INDEX = 0xFFFFFFFF
};


////////////////////////////////////////////////////////////////////////////////////////////////
//	IAttachedDataContainer
///	the interface for an attachment-data-container.
/**
*	In order to use an attachment-data-container you have to supply several Typedefs.
*	Take a look at the template-derivate AttachmentDataContainer<T> to see wich typedefs
	and operations have to be supplied in addition to the interface-methods.
*	if possible you should use the derivate-class AttachmentDataContainer<T> instead creating
	your own derivate of IAttachedDataContainer.
*/
class IAttachmentDataContainer
{
	public:
		virtual ~IAttachmentDataContainer()		{}

		virtual void resize(size_t iSize) = 0;///< resize the data array
		virtual size_t size() = 0;///< returns the size of the data-array.
		virtual void copy_data(size_t indFrom, size_t indTo) = 0;///< copy data from entry indFrom to entry indTo.
		virtual void reset_entry(size_t index) = 0;///< resets the entry to its default value.
		
	/**	copies entrys from the this-container to the specified target container.
	 *	For the i-th entry in the dest-container, pIndexMap has to contain
	 *	the index of the associated source entry in this container.
	 *	Num specifies the number of entries to be copied.
	 *	Make sure, that pDest can hold 'num' elements.
	 *
	 *	pDestCon has to have the same or a derived dynamic type as the container
	 *	on which this method is called.*/
		virtual void copy_to_container(IAttachmentDataContainer* pDestCon,
									   int* indexMap, int num) const = 0;
		
	/**
	*	defragment should clear the containers data from unused entries.
	*	pNewIndices should be an array of indices, wich holds a new index for each entry in IAttachedDataContainer.
	*	pNewIndices has thus to hold as many indices as there are entries in IAttachedDataContainer.
	*	If an entry shall not appear in the defragmented container, its new index has to be set to INVALID_ATTACHMENT_INDEX.
	*	numValidElements has to specify the size of the defragmented container - it thus has to equal the number of valid indices in pNewIndices.
	*/
		virtual void defragment(size_t* pNewIndices, size_t numValidElements) = 0;

	///	returns the size in bytes, which the container occupies
	/**
	 * Mainly for debugging purposes.
	 */
		virtual size_t occupied_memory() = 0;
};



/* THOUGHTS
*	AttachmentDataContainer<T> should probably be defined in another header, since it is somehow specialised for libGrid.
*	same goes for Attachment<T>
*/
////////////////////////////////////////////////////////////////////////////////////////////////
//	AttachedDataContainer
///	A generic specialization of IAttachedDataContainer.
/**
*	This template-class not only simplifies the creation of custom containers,
	it also defines some types, operators and values, which are essential to use an AttachmentDataContainer with libGrid.
	In particular libGrids AttachmentAccessors require these definitions.
*/
template <class T> class AttachmentDataContainer : public IAttachmentDataContainer
{
	private:
		typedef AttachmentDataContainer<T>	ClassType;
		//typedef PageContainer<T>			DataContainer;
		typedef std::vector<T>				DataContainer;

	public:
		typedef T	ValueType;

		AttachmentDataContainer()	:	m_bDefaultValueSet(false)	{}
		AttachmentDataContainer(const T& defaultValue)	: m_bDefaultValueSet(true), m_defaultValue(defaultValue)	{}

		virtual ~AttachmentDataContainer()			{m_vData.clear();}

		virtual void resize(size_t iSize)
			{
				if(iSize > 0)
				{
					if(m_bDefaultValueSet)
						m_vData.resize(iSize, m_defaultValue);
					else
						m_vData.resize(iSize);
				}
				else
					m_vData.clear();
			}

		virtual size_t size()	{return m_vData.size();}

		virtual void copy_data(size_t indFrom, size_t indTo)    {m_vData[indTo] = m_vData[indFrom];}

		virtual void reset_entry(size_t index)
			{
				if(m_bDefaultValueSet)
					m_vData[index] = m_defaultValue;
				else
					m_vData[index] = ValueType();
			}

		virtual void defragment(size_t* pNewIndices, size_t numValidElements)
			{
				size_t numOldElems = size();
				DataContainer vDataOld = m_vData;
				m_vData.resize(numValidElements);
				for(size_t i = 0; i < numOldElems; ++i)
				{
					size_t nInd = pNewIndices[i];
					if(nInd != INVALID_ATTACHMENT_INDEX)
						m_vData[nInd] = vDataOld[i];
				}
			}
	
	/**	copies entrys from the this-container to the container
	 *	specified by pDestCon.
	 *	For the i-th entry in the target container, pIndexMap has to contain
	 *	the index of the associated source entry in this container.
	 *	Num specifies the number of entries to be copied.
	 *	Make sure, that pDest can hold 'num' elements.
	 *
	 *	pDestCon has to have the same or a derived dynamic type as the container
	 *	on which this method is called.*/
		virtual void copy_to_container(IAttachmentDataContainer* pDestCon,
										int* indexMap, int num) const
			{
				ClassType* destConT = dynamic_cast<ClassType*>(pDestCon);
				assert(destConT && "Type of pDestBuf has to be the same as the"
						"type of this buffer");

				if(!destConT)
					return;

				DataContainer& dest = destConT->get_data_container();
				for(int i = 0; i < num; ++i)
					dest[i] = m_vData[indexMap[i]];
			}


	///	returns the memory occupied by the container
		virtual size_t occupied_memory()
		{
			return m_vData.capacity() * sizeof(T);
		}

		inline const T& get_elem(size_t index) const      {return m_vData[index];}
		inline T& get_elem(size_t index)                  {return m_vData[index];}
		inline const T& operator[] (size_t index) const	{return m_vData[index];}
		inline T& operator[] (size_t index)				{return m_vData[index];}

	///	swaps the buffer content of associated data
		void swap(AttachmentDataContainer<T>& container)	{m_vData.swap(container.m_vData);}

	protected:
		DataContainer& get_data_container()			{return m_vData;}
		//inline const T* get_ptr() const			{return &m_vData.front();}
		//inline T* get_ptr()						{return &m_vData.front();}
		
	protected:
		DataContainer	m_vData;
		bool			m_bDefaultValueSet;
		T				m_defaultValue;
};

////////////////////////////////////////////////////////////////////////////////////////////////
//	IAttachment
///	the interface for attachments.
/**
*	Attachments can be attached to an AttachmentPipe and thus enhance the pipes elements by data,
	whose type, container and behavior is defined by the Attachment itself.
*	In order to use an Attachment with libGrid (in particular with libGrids AttachmentAccessors),
	derivatives of IAttachment have to feature some special typedefs (see Attachment<T> for more information).
*	Whenever possible you should use the template-derivative Attachment<T> instead of IAttachment.
*/
class IAttachment : public UID
{
	public:
		IAttachment() : m_name("")   {}
		IAttachment(const char* name) : m_name(name)
			{assert(m_name);}

		virtual ~IAttachment()  {}
		virtual IAttachment* clone() = 0;
		virtual IAttachmentDataContainer*	create_container() = 0;
		virtual bool default_pass_on_behaviour() const = 0;

		const char* get_name()  {return m_name;}    ///< should only be used for debug purposes.

	protected:
		const char* m_name; //only for debug
};

////////////////////////////////////////////////////////////////////////////////////////////////
//	Attachment
/// A generic specialization of IAttachment
/**
*	This class is intended to simplify the process of Attachment creation.
*	Note that there are typedefs, which are required by libGrids AttachmentAccessors.
*/
template <class T> class Attachment : public IAttachment
{
	public:
		typedef AttachmentDataContainer<T>	ContainerType;
		typedef T							ValueType;

		Attachment() : IAttachment(), m_passOnBehaviour(false)    {}
		Attachment(bool passOnBehaviour) : IAttachment(), m_passOnBehaviour(passOnBehaviour)    {}
		Attachment(const char* name) : IAttachment(name), m_passOnBehaviour(false)   		{}
		Attachment(const char* name, bool passOnBehaviour) : IAttachment(name), m_passOnBehaviour(passOnBehaviour)	{}

		virtual ~Attachment()	{}
		virtual IAttachment* clone()							{IAttachment* pA = new Attachment<T>; *pA = *this; return pA;}
		virtual IAttachmentDataContainer* create_container()	{return new ContainerType;}
		virtual bool default_pass_on_behaviour() const			{return m_passOnBehaviour;}
		IAttachmentDataContainer* create_container(const T& defaultValue)	{return new ContainerType(defaultValue);}

	protected:
		bool m_passOnBehaviour;
};

////////////////////////////////////////////////////////////////////////////////////////////////
//	AttachmentEntry
///	This struct is used by AttachmentPipe in order to manage its attachments
struct AttachmentEntry
{
	AttachmentEntry() : m_pAttachment(NULL), m_pContainer(NULL), m_userData(0)	{}
	AttachmentEntry(IAttachment* pAttachment, IAttachmentDataContainer* pContainer, uint userData = 0) :
			m_pAttachment(pAttachment), m_pContainer(pContainer), m_userData(userData)	{}

	IAttachment*				m_pAttachment;
	IAttachmentDataContainer*	m_pContainer;
	uint						m_userData;
};


////////////////////////////////////////////////////////////////////////////////////////////////
///	attachment_traits define the interface that enables you to use your own data-types with the AttachmentPipe.
/**
 * Perform template-specialization for your own data-types and their respective handlers.
 */
template<class TElem, class TElemHandler>
class attachment_traits
{
	public:
		typedef TElem&		ElemRef;
		typedef void*		ElemPtr;
		typedef const void* ConstElemPtr;
		typedef void*		ElemHandlerPtr;
		typedef const void*	ConstElemHandlerPtr;

	///	mark the element as invalid.
	/**	You may do something like elem = NULL, if TElem is a pointer type.*/
		static inline void invalidate_entry(ElemHandlerPtr pHandler, ElemRef elem)				{/*STATIC_ASSERT(0, INVALID_ATTACHMENT_TRAITS);*/}
		static inline bool entry_is_invalid(ElemHandlerPtr pHandler, ElemRef elem)				{return true;/*STATIC_ASSERT(0, INVALID_ATTACHMENT_TRAITS);*/}
		static inline uint get_data_index(ElemHandlerPtr pHandler, ConstElemPtr elem)			{return INVALID_ATTACHMENT_INDEX;/*STATIC_ASSERT(0, INVALID_ATTACHMENT_TRAITS);*/}
		static inline void set_data_index(ElemHandlerPtr pHandler, ElemPtr elem, size_t index)	{/*STATIC_ASSERT(0, INVALID_ATTACHMENT_TRAITS);*/}
};


////////////////////////////////////////////////////////////////////////////////////////////////
//	AttachmentPipe
///	Handles data which has been attached to the pipe using callbacks for the element.
/**
 * The AttachmentPipe can be used to attach data to a collection of elements.
 * Elements have to be registered at the AttachmentPipe. Using the methods
 * defined in the attachment_traits template class, registered elements are
 * associated with their data-entries.
 *
 * TODO:
 * enum Options : CopyAllowed
 *
 * - copy_values(TElem from, TElem to)
 * - swap_entry_indices()?!?
 */
template<class TElem, class TElemHandler>
class AttachmentPipe
{
	public:
		typedef TElem								element;
		typedef TElemHandler						ElementHandler;
		typedef std::list<AttachmentEntry>			AttachmentEntryContainer;
		typedef AttachmentEntryContainer::iterator	AttachmentEntryIterator;
		typedef AttachmentEntryContainer::const_iterator	ConstAttachmentEntryIterator;
		typedef Hash<AttachmentEntryIterator, uint>		AttachmentEntryIteratorHash;
		typedef attachment_traits<TElem, TElemHandler>	atraits;

	public:
		AttachmentPipe();
		AttachmentPipe(typename atraits::ElemHandlerPtr pHandler);
		~AttachmentPipe();
		
		inline typename atraits::ElemHandlerPtr get_elem_handler()		{return m_pHandler;}
		
	///	calls both clear_elements and clear_attachments.
		void clear();
	///	clears elements and associated data but keeps registered attachments
		void clear_elements();
	///	clears registered attachments and associated data but keeps existing elements.
		void clear_attachments();

	///	Reserves memory for element- and data-entries.
	/**	This method reserves memory for data and elements.
	 * Note that numElems specifies the total amount of elements
	 * for which data shall be reserved. If numElems is smaller than
	 * num_elements, then nothing will be done.
	 * Note that a call to reserve does not change num_elements nor
	 * num_data_entries.
	 *
	 * If you want to reserve space for additional N elements, please
	 * call reserve with numElems = num_elements() + N.
	 *
	 * Note that reserve is optional and only serves performance benefits.
	 */
		void reserve(size_t numElems);

	///	Registers a new element at the attachment pipe.
	/**	Registers the element and reserves memory for all registered
	 * attachments. Note that this method may be faster if memory has
	 * previously been reserved using reserve_new.
	 */
		void register_element(TElem elem);

	///	Unregisters the given element.
	/**	Unregisters the given element but does not yet clear associated data.
	 * Indeed the memory is kept until defragment is called or new elements
	 * reuse the memory.
	 */
		void unregister_element(const TElem& elem);

	/**	Aligns data with elements and removes unused data-memory.*/
		void defragment();

	/**\brief attaches a new data-array to the pipe.
	 *
	 * Attachs a new attachment and creates a container which holds the
	 * data associated with the elements through this attachment.
	 * Several overloads exist: You may pass an attachment together with
	 * a default value and with an option constant or simply the attachment
	 * together with an optional option constant. Not that in the first
	 * case the concrete type of the attachment has to be known, whereas
	 * in the second case only the interface IAttachment has to be specified.
	 *
	 * The option constant is not used by the attachment system but may be
	 * used by a user to store a constant with each attachment.
	 * \{
	 */
		template <class TAttachment>
		void attach(TAttachment& attachment,
					const typename TAttachment::ValueType& defaultValue,
					uint options);

		void attach(IAttachment& attachment, uint options = 0);
	/**	\}	*/

	///	Removes the data associated with the given attachment from the pipe.
		void detach(IAttachment& attachment);

	///	Returns true if the given attachment is currently attached to the pipe.
		bool has_attachment(IAttachment& attachment) const;

	///	Lets you access the raw data array associated with the given attachment.
	/**	If you access several arrays through this method, it is guaranteed that
	 * the data associated with one specific object are found at the same indices
	 * in those arrays.
	 *
	 * Note that if you access the data arrays after a call to defragment, then
	 * the i-th data-entry corresponds to the i-th element.
	 *
	 * Please not that the pointer may be invalidated through the following operations:
	 * 		- register_element
	 * 		- defragment
	 * 		- clear, clear_elements, clear_attachments
	 */
		template <class TAttachment>
		typename TAttachment::ValueType*
		get_data_array(TAttachment& attachment);
		
	/**	\brief Returns the data container managing the data array for the given attachment.
	 * \{ */
		IAttachmentDataContainer* get_data_container(IAttachment& attachment) const;

		template <class TAttachment>
		typename TAttachment::ContainerType*
		get_data_container(TAttachment& attachment);
	/**	\} */

		inline ConstAttachmentEntryIterator attachments_begin() const	{return m_attachmentEntryContainer.begin();}
		inline ConstAttachmentEntryIterator attachments_end() const		{return m_attachmentEntryContainer.end();}

	///	Returns the number of registered elements
		inline size_t num_elements() const		{return m_numElements;}
	///	Returns the size of the associated data arrays
	/**	Note: If the pipe is fragmented, then num_elements and num_data_entries
	 * differ. If the pipe however isn't fragmented, then both values are the same.
	 */
		inline size_t num_data_entries() const	{return m_vEntries.size();}

	///	Returns whether the attachment pipe is fragmented.
	/**	The pipe gets fragmented whenever elements are erased.
	 * Through the creation of new elements the pipe may be automatically
	 * defragmented.
	 * Through a call to defragment() the pipe can be defragmented at any time.
	 */
		inline bool is_fragmented() const		{return m_numElements != m_vEntries.size();}

		void reset_values(size_t dataIndex);///< fills the attached data at the given index with the default values.

	protected:
		void resize_attachment_containers(size_t newSize);
		void grow_attachment_containers(size_t newMinSize);
		inline size_t get_container_size();

	protected:
		typedef std::vector<TElem>		ElemEntryVec;
		typedef std::stack<size_t>		UINTStack;

	protected:
		AttachmentEntryContainer	m_attachmentEntryContainer;
		AttachmentEntryIteratorHash	m_attachmentEntryIteratorHash;

		ElemEntryVec	m_vEntries;	///< same size as attachment containers.
		UINTStack		m_stackFreeEntries;	///< holds indices to free entries.

		size_t			m_numElements;
		size_t			m_containerSize; ///< total size of containers.
		
		typename atraits::ElemHandlerPtr	m_pHandler;
};


////////////////////////////////////////////////////////////////////////////////////////////////
//	AttachmentAccessor
///	Used to access data that has been attached to an attachment pipe.
/**
 * Once initialized (use the constructor), an AttachmentAccessor can be used to access
 * the data stored in the given AttachmentPipe
 */
template <class TElem, class TAttachment, class TElemHandler>
class AttachmentAccessor
{
	public:
		typedef TElem								element;
		typedef typename TAttachment::ValueType		ValueType;
		typedef typename TAttachment::ContainerType	ContainerType;
		typedef attachment_traits<TElem, TElemHandler>	atraits;

	public:
		AttachmentAccessor();
		AttachmentAccessor(const AttachmentAccessor& aa);
		AttachmentAccessor(AttachmentPipe<TElem, TElemHandler>& attachmentPipe, TAttachment& attachment);

		void access(AttachmentPipe<TElem, TElemHandler>& attachmentPipe, TAttachment& attachment);

		inline ValueType&
		operator[](typename atraits::ConstElemPtr elem)
			{
				assert((attachment_traits<TElem, TElemHandler>::get_data_index(m_pHandler, elem) != INVALID_ATTACHMENT_INDEX) &&
						"ERROR in AttachmentAccessor::operator[]: accessing element with invalid attachment index!");
				assert(m_pContainer && "ERROR in AttachmentAccessor::operator[]: no AttachmentPipe assigned.");
				return m_pContainer->get_elem(attachment_traits<TElem, TElemHandler>::get_data_index(m_pHandler, elem));
			}
			
		inline const ValueType&
		operator[](typename atraits::ConstElemPtr elem) const
			{
				assert((attachment_traits<TElem, TElemHandler>::get_data_index(m_pHandler, elem) != INVALID_ATTACHMENT_INDEX) &&
						"ERROR in AttachmentAccessor::operator[]: accessing element with invalid attachment index!");
				assert(m_pContainer && "ERROR in AttachmentAccessor::operator[]: no AttachmentPipe assigned.");
				return m_pContainer->get_elem(attachment_traits<TElem, TElemHandler>::get_data_index(m_pHandler, elem));
			}
			
/*
		inline ValueType&
		operator[](int index)
			{
				assert(m_pContainer && "ERROR in AttachmentAccessor::operator[]: no AttachmentPipe assigned.");
				return m_pContainer->get_elem(index);
			}
*/
		inline bool valid()
			{return m_pContainer != NULL;}

		inline void invalidate()
			{m_pContainer = NULL;}
			
	///	calls swap on associated containers
		void swap(AttachmentAccessor<TElem, TAttachment, TElemHandler>& acc)
		{
			m_pContainer->swap(*acc.m_pContainer);
		}

	protected:
		ContainerType*	m_pContainer;
		TElemHandler*	m_pHandler;
};


}//	end of namespace

//	include implementation
#include "attachment_pipe.hpp"

#endif
