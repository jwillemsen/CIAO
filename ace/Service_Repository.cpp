// $Id$

#include "ace/Service_Repository.h"

#if !defined (__ACE_INLINE__)
#include "ace/Service_Repository.inl"
#endif /* __ACE_INLINE__ */

#include "ace/Service_Types.h"
#include "ace/Object_Manager.h"
#include "ace/Log_Msg.h"
#include "ace/ACE.h"
#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_errno.h"
#include "ace/OS_NS_string.h"

ACE_RCSID (ace,
           Service_Repository,
           "$Id$")

  ACE_BEGIN_VERSIONED_NAMESPACE_DECL

ACE_ALLOC_HOOK_DEFINE(ACE_Service_Repository)

// Process-wide Service Repository.
  ACE_Service_Repository *ACE_Service_Repository::svc_rep_ = 0;

// Controls whether the Service_Repository is deleted when we shut
// down (we can only delete it safely if we created it)!
int ACE_Service_Repository::delete_svc_rep_ = 0;

void
ACE_Service_Repository::dump (void) const
{
#if defined (ACE_HAS_DUMP)
  ACE_TRACE ("ACE_Service_Repository::dump");
#endif /* ACE_HAS_DUMP */
}

ACE_Service_Repository::ACE_Service_Repository (void)
  : service_vector_ (0),
    current_size_ (0),
    total_size_ (0)
{
  ACE_TRACE ("ACE_Service_Repository::ACE_Service_Repository");
}

ACE_Service_Repository *
ACE_Service_Repository::instance (int size /* = ACE_Service_Repository::DEFAULT_SIZE */)
{
  ACE_TRACE ("ACE_Service_Repository::instance");

  if (ACE_Service_Repository::svc_rep_ == 0)
    {
      // Perform Double-Checked Locking Optimization.
      ACE_MT (ACE_GUARD_RETURN (ACE_Recursive_Thread_Mutex, ace_mon,
                                *ACE_Static_Object_Lock::instance (), 0));
      if (ACE_Service_Repository::svc_rep_ == 0)
        {
          if (ACE_Object_Manager::starting_up () ||
              !ACE_Object_Manager::shutting_down ())
            {
              ACE_NEW_RETURN (ACE_Service_Repository::svc_rep_,
                              ACE_Service_Repository (size),
                              0);
              ACE_Service_Repository::delete_svc_rep_ = 1;
            }
        }
    }

  //  ACE_ASSERT (ACE_Service_Repository::svc_rep_ != 0);
  return ACE_Service_Repository::svc_rep_;
}

ACE_Service_Repository *
ACE_Service_Repository::instance (ACE_Service_Repository *s)
{
  ACE_TRACE ("ACE_Service_Repository::instance");
  ACE_MT (ACE_GUARD_RETURN (ACE_Recursive_Thread_Mutex, ace_mon,
                            *ACE_Static_Object_Lock::instance (), 0));

  ACE_Service_Repository *t = ACE_Service_Repository::svc_rep_;
  // We can't safely delete it since we don't know who created it!
  ACE_Service_Repository::delete_svc_rep_ = 0;

  ACE_Service_Repository::svc_rep_ = s;
  return t;
}

void
ACE_Service_Repository::close_singleton (void)
{
  ACE_TRACE ("ACE_Service_Repository::close_singleton");

  ACE_MT (ACE_GUARD (ACE_Recursive_Thread_Mutex, ace_mon,
                     *ACE_Static_Object_Lock::instance ()));

  if (ACE_Service_Repository::delete_svc_rep_)
    {
      delete ACE_Service_Repository::svc_rep_;
      ACE_Service_Repository::svc_rep_ = 0;
      ACE_Service_Repository::delete_svc_rep_ = 0;
    }
}

// Initialize the Repository to a clean slate.

int
ACE_Service_Repository::open (int size)
{
  ACE_TRACE ("ACE_Service_Repository::open");

  ACE_Service_Type **temp;

  ACE_NEW_RETURN (temp,
                  ACE_Service_Type *[size],
                  -1);

  this->service_vector_ = const_cast<const ACE_Service_Type **> (temp);
  this->total_size_ = size;
  return 0;
}

ACE_Service_Repository::ACE_Service_Repository (int size)
  : current_size_ (0)
{
  ACE_TRACE ("ACE_Service_Repository::ACE_Service_Repository");

  if (this->open (size) == -1)
    ACE_ERROR ((LM_ERROR,
                ACE_LIB_TEXT ("%p\n"),
                ACE_LIB_TEXT ("ACE_Service_Repository")));
}

// Finalize (call <fini> and possibly delete) all the services.

int
ACE_Service_Repository::fini (void)
{
  ACE_TRACE ("ACE_Service_Repository::fini");
  ACE_MT (ACE_GUARD_RETURN (ACE_Recursive_Thread_Mutex, ace_mon, this->lock_, -1));
  int retval = 0;

  if (this->service_vector_ != 0)
    {
      // <fini> the services in reverse order.  Note that if services
      // were removed from the middle of the repository the order
      // won't necessarily be maintained since the <remove> method
      // performs compaction.  However, the common case is not to
      // remove services, so typically they are deleted in reverse
      // order.

      for (int i = this->current_size_ - 1; i >= 0; i--)
  {
    ACE_Service_Type *s =
      const_cast<ACE_Service_Type *> (this->service_vector_[i]);

    if (ACE::debug ())
      {
        ACE_DEBUG ((LM_DEBUG,
        ACE_LIB_TEXT ("(%P|%t) SR::fini, %@ [%d] (%d): "),
        this, i, this->total_size_));
        s->dump();
      }

    // Collect any errors.
    int ret = s->fini ();
    if (ACE::debug ()>1)
      {
        ACE_DEBUG ((LM_DEBUG,
        ACE_LIB_TEXT ("(%P|%t) SR::fini, returned %d\n"),
        ret));
      }
    retval += ret;
  }
    }

  return (retval == 0) ? 0 : -1;
}

// Close down all the services.

int
ACE_Service_Repository::close (void)
{
  ACE_TRACE ("ACE_Service_Repository::close");
  ACE_MT (ACE_GUARD_RETURN (ACE_Recursive_Thread_Mutex, ace_mon, this->lock_, -1));

  if (this->service_vector_ != 0)
    {
      // Delete services in reverse order.  Note that if services were
      // removed from the middle of the repository the order won't
      // necessarily be maintained since the <remove> method performs
      // compaction.  However, the common case is not to remove
      // services, so typically they are deleted in reverse order.

      if(ACE::debug ())
        ACE_DEBUG ((LM_DEBUG,
                    ACE_LIB_TEXT ("(%P|%t) SR::close, this=%@, size=%d\n"),
                    this,
                    this->current_size_));

      for (int i = this->current_size_ - 1; i >= 0; i--)
        {
          if(ACE::debug ())
            ACE_DEBUG ((LM_DEBUG,
                        ACE_LIB_TEXT ("(%P|%t) SR::close, this=%@, delete so[%d]=%@ (%s)\n"),
                        this,
                        i,
                        this->service_vector_[i],
                        this->service_vector_[i]->name ()));

          ACE_Service_Type *s = const_cast<ACE_Service_Type *> (this->service_vector_[i]);
          --this->current_size_;
          delete s;
        }

      delete [] this->service_vector_;
      this->service_vector_ = 0;
      this->current_size_ = 0;
    }

  return 0;
}

ACE_Service_Repository::~ACE_Service_Repository (void)
{
  ACE_TRACE ("ACE_Service_Repository::~ACE_Service_Repository");
  if(ACE::debug ())
    ACE_DEBUG ((LM_DEBUG, "(%P|%t) SR::<dtor>, this=%@\n", this));
  this->close ();
}

// Locate an entry with <name> in the table.  If <ignore_suspended> is
// set then only consider services marked as resumed.  If the caller
// wants the located entry, pass back a pointer to the located entry
// via <srp>.  If <name> is not found -1 is returned.  If <name> is
// found, but it is suspended and the caller wants to ignore suspended
// services a -2 is returned.  Must be called with locks held.

int
ACE_Service_Repository::find_i (const ACE_TCHAR name[],
                                const ACE_Service_Type **srp,
                                int ignore_suspended) const
{
  ACE_TRACE ("ACE_Service_Repository::find_i");
  int i;

  for (i = 0; i < this->current_size_; i++)
    if (ACE_OS::strcmp (name,
                        this->service_vector_[i]->name ()) == 0)
      break;

  if (i < this->current_size_)
    {
      if (this->service_vector_[i]->fini_called ())
        {
          if (srp != 0)
            *srp = 0;
          return -1;
        }

      if (srp != 0)
        *srp = this->service_vector_[i];
      if (ignore_suspended
          && this->service_vector_[i]->active () == 0)
        return -2;
      return i;
    }
  else
    return -1;
}

int
ACE_Service_Repository::find (const ACE_TCHAR name[],
                              const ACE_Service_Type **srp,
                              int ignore_suspended) const
{
  ACE_TRACE ("ACE_Service_Repository::find");
  ACE_MT (ACE_GUARD_RETURN (ACE_Recursive_Thread_Mutex, ace_mon, this->lock_, -1));

  return this->find_i (name, srp, ignore_suspended);
}


// Insert the ACE_Service_Type SR into the repository.  Note that
// services may be inserted either resumed or suspended. Using same name
// as in an existing service causes the delete () to be called for the old one,
// i.e. make sure @code sr is allocated on the heap!

int
ACE_Service_Repository::insert (const ACE_Service_Type *sr)
{
  ACE_TRACE ("ACE_Service_Repository::insert");

  int return_value = -1;
  ACE_Service_Type *s = 0;

  {
    // @TODO: Do we need a recursive mutex here?
    ACE_MT (ACE_GUARD_RETURN (ACE_Recursive_Thread_Mutex, ace_mon, this->lock_, -1));
    int i;

    // Check to see if this is a duplicate.
    for (i = 0; i < this->current_size_; i++)
      if (ACE_OS::strcmp (sr->name (),
                          this->service_vector_[i]->name ()) == 0)
        break;

    // Replacing an existing entry
    if (i < this->current_size_)
      {
  return_value = 0;
  // Check for self-assignment...
  if (sr != this->service_vector_[i])
    {
      s = const_cast<ACE_Service_Type *> (this->service_vector_[i]);
      this->service_vector_[i] = sr;
    }
      }
    // Adding a new entry.
    else if (i < this->total_size_)
      {
  this->service_vector_[i] = sr;
  this->current_size_++;
  return_value = 0;
      }

    if (ACE::debug ())
      {
  ACE_DEBUG ((LM_DEBUG,
        "(%P|%t) SR::insert, repo=%@ [%d] (size=%d): ",
        this,
        i,
        this->total_size_));
  sr->dump();
      }
  }

  // Delete outside the lock
  if (s != 0)
    {
      if (ACE::debug () > 1)
  {
    ACE_DEBUG ((LM_DEBUG,
          "(%P|%t) SR::insert, repo=%@ - destroying : ",
          this));
    s->dump();
  }
      delete s;
    }

  if (return_value == -1)
    ACE_OS::last_error (ENOSPC);

  return return_value;
}

// Re-resume a service that was previously suspended.

int
ACE_Service_Repository::resume (const ACE_TCHAR name[],
                                const ACE_Service_Type **srp)
{
  ACE_TRACE ("ACE_Service_Repository::resume");
  ACE_MT (ACE_GUARD_RETURN (ACE_Recursive_Thread_Mutex, ace_mon, this->lock_, -1));

  int i = this->find_i (name, srp, 0);

  if (i == -1)
    return -1;

  return this->service_vector_[i]->resume ();
}

// Suspend a service so that it will not be considered active under
// most circumstances by other portions of the ACE_Service_Repository.

int
ACE_Service_Repository::suspend (const ACE_TCHAR name[],
                                 const ACE_Service_Type **srp)
{
  ACE_TRACE ("ACE_Service_Repository::suspend");
  ACE_MT (ACE_GUARD_RETURN (ACE_Recursive_Thread_Mutex, ace_mon, this->lock_, -1));
  int i = this->find_i (name, srp, 0);

  if (i == -1)
    return -1;

  return this->service_vector_[i]->suspend ();
}


/**
 * @brief Completely remove a <name> entry from the Repository and
 * dynamically unlink it if it was originally dynamically linked.
 *
 * Since the order of services in the Respository matters, we can't
 * simply overwrite the entry being deleted with the last and
 * decrement the <current_size> by 1 - we must "pack" the array.  A
 * good example of why the order matters is a dynamic service, in
 * whose DLL there is at least one static service. In order to prevent
 * SEGV during finalization, those static services must be finalized
 * _before_the dynamic service that owns them. Otherwice the TEXT
 * segment, containing the code for the static service's desructor may
 * be unloaded with the DLL.
 */

int
ACE_Service_Repository::remove (const ACE_TCHAR name[], ACE_Service_Type **ps)
{
  ACE_TRACE ("ACE_Service_Repository::remove");
  ACE_Service_Type *s = 0;
  {
    ACE_MT (ACE_GUARD_RETURN (ACE_Recursive_Thread_Mutex, ace_mon, this->lock_, -1));
    int i = this->find_i (name, 0, 0);

    // Not found
    if (i == -1)
      return -1;

    // We need the old ptr to be delete outside the lock
    s = const_cast<ACE_Service_Type *> (this->service_vector_[i]);

    // Pack the array
    --this->current_size_;
    for (int j = i; j < this->current_size_; j++)
      this->service_vector_[j] = this->service_vector_[j+1];
  }

  if (ps != 0)
    *ps = s;
  else
    delete s;
  return 0;
}

ACE_ALLOC_HOOK_DEFINE(ACE_Service_Repository_Iterator)

  void
ACE_Service_Repository_Iterator::dump (void) const
{
#if defined (ACE_HAS_DUMP)
  ACE_TRACE ("ACE_Service_Repository_Iterator::dump");
#endif /* ACE_HAS_DUMP */
}

// Initializes the iterator and skips over any suspended entries at
// the beginning of the table, if necessary.  Note, you must not
// perform destructive operations on elements during this iteration...

ACE_Service_Repository_Iterator::ACE_Service_Repository_Iterator
(ACE_Service_Repository &sr,
 int ignr_suspended)
  : svc_rep_ (sr),
    next_ (-1),
    ignore_suspended_ (ignr_suspended)
{
  this->advance ();
}

// Obtains a pointer to the next valid service in the table.  If there
// are no more entries, returns 0, else 1.

int
ACE_Service_Repository_Iterator::next (const ACE_Service_Type *&sr)
{
  ACE_TRACE ("ACE_Service_Repository_Iterator::next");
  if (this->next_ < this->svc_rep_.current_size_)
    {
      sr = this->svc_rep_.service_vector_[this->next_];
      return 1;
    }
  else
    return 0;
}

int
ACE_Service_Repository_Iterator::done (void) const
{
  ACE_TRACE ("ACE_Service_Repository_Iterator::done");

  return this->next_ >= this->svc_rep_.current_size_;
}

// Advance the iterator by the proper amount.  If we are ignoring
// suspended entries and the current entry is suspended, then we must
// skip over this entry.  Otherwise, we must advance the NEXT index to
// reference the next valid service entry.

int
ACE_Service_Repository_Iterator::advance (void)
{
  ACE_TRACE ("ACE_Service_Repository_Iterator::advance");

  for (++this->next_;
       this->next_ < this->svc_rep_.current_size_
   && this->ignore_suspended_
   && this->svc_rep_.service_vector_[this->next_]->active () == 0;
       this->next_++)
    continue;

  return this->next_ < this->svc_rep_.current_size_;
}

ACE_END_VERSIONED_NAMESPACE_DECL
