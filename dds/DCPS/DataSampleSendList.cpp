/*
 * $Id$
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include "DCPS/DdsDcps_pch.h" //Only the _pch include should start with DCPS/
//#include "DataSampleList.h"
#include "DataSampleSendList.h"
#include "DataSampleListElement.h"
#include "Definitions.h"
#include "PublicationInstance.h"

#include "dds/DCPS/transport/framework/TransportSendListener.h"

#if !defined (__ACE_INLINE__)
#include "DataSampleSendList.inl"
#endif /* __ACE_INLINE__ */

namespace OpenDDS {
namespace DCPS {


const DataSampleSendList*
DataSampleSendList::send_list_containing_element(const DataSampleListElement* element,
                                                 std::vector<DataSampleSendList*> send_lists)
{
  DataSampleListElement* head = const_cast<DataSampleListElement*>(element);

  while (head->previous_send_sample_ != 0) {
    head = head->previous_send_sample_;
  }

  DataSampleSendList* list_containing_element = 0;

  for(std::vector<DataSampleSendList*>::iterator it = send_lists.begin(); it != send_lists.end(); ++it) {
    if ((*it)->head_ == head) {
    	list_containing_element = *it;
    	break;
    }
  }
  return list_containing_element;
}


bool
DataSampleSendList::dequeue(const DataSampleListElement* stale)
{
  if (head_ == 0) {
    return false;
  }

  // Same as dequeue from head.
  if (stale == head_) {
    DataSampleListElement* tmp = head_;
    return dequeue_head(tmp);
  }

  // Search from head_->next_send_sample_.
  DataSampleListElement* toRemove = 0;
  for (DataSampleListElement* item = head_->next_send_sample_;
       item != 0 && toRemove == 0;
       item = item->next_send_sample_) {
    if (item == stale) {
      toRemove = item;
    }
  }

  if (toRemove) {
    size_ --;
    // Remove from the previous element.
    toRemove->previous_send_sample_->next_send_sample_ = toRemove->next_send_sample_ ;

    // Remove from the next element.
    if (toRemove->next_send_sample_ != 0) {
      // Remove from the inside of the list.
      toRemove->next_send_sample_->previous_send_sample_ = toRemove->previous_send_sample_ ;

    } else {
      toRemove->previous_send_sample_->next_send_sample_ = 0;
      // Remove from the tail of the list.
      tail_ = toRemove->previous_send_sample_ ;
    }

    toRemove->next_send_sample_ = 0;
    toRemove->previous_send_sample_ = 0;
  }

  return toRemove;
}

void
DataSampleSendList::enqueue_tail(DataSampleSendList list)
{
  //// Make the appended list linked with next_send_sample_ first.
  //DataSampleListElement* cur = list.head_;

  //if (list.size_ > 1 && cur->next_send_sample_ == 0)
  // {
  //   for (ssize_t i = 0; i < list.size_; i ++)
  //     {
  //       cur->next_send_sample_ = cur->next_sample_;
  //       cur = cur->next_sample_;
  //     }
  // }

  if (head_ == 0) {
    head_ = list.head_;
    tail_ = list.tail_;
    size_ = list.size_;

  } else {
    tail_->next_send_sample_
    //= tail_->next_sample_
    = list.head_;
    list.head_->previous_send_sample_ = tail_;
    //list.head_->previous_sample_ = tail_;
    tail_ = list.tail_;
    size_ = size_ + list.size_;
  }
}

// -----------------------------------------------

DataSampleSendListIterator::DataSampleSendListIterator(
  DataSampleListElement* head,
  DataSampleListElement* tail,
  DataSampleListElement* current)
  : head_(head)
  , tail_(tail)
  , current_(current)
{
}

DataSampleSendListIterator&
DataSampleSendListIterator::operator++()
{
  if (this->current_)
    this->current_ = this->current_->next_send_sample_;

  return *this;
}

DataSampleSendListIterator
DataSampleSendListIterator::operator++(int)
{
  DataSampleSendListIterator tmp(*this);
  ++(*this);
  return tmp;
}

DataSampleSendListIterator&
DataSampleSendListIterator::operator--()
{
  if (this->current_)
    this->current_ = this->current_->previous_send_sample_;

  else
    this->current_ = this->tail_;

  return *this;
}

DataSampleSendListIterator
DataSampleSendListIterator::operator--(int)
{
  DataSampleSendListIterator tmp(*this);
  --(*this);
  return tmp;
}

DataSampleSendListIterator::reference
DataSampleSendListIterator::operator*()
{
  // Hopefully folks will be smart enough to not dereference a
  // null iterator.  Such a case should only exist for an "end"
  // iterator.  Otherwise we may want to throw an exception here.
  // assert (this->current_ != 0);

  return *(this->current_);
}

DataSampleSendListIterator::pointer
DataSampleSendListIterator::operator->()
{
  return this->current_;
}

} // namespace DCPS
} // namespace OpenDDS