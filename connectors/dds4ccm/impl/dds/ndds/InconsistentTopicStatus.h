/**
 * @author William R. Otte <wotte@dre.vanderbilt.edu>
 * @author Johnny Willemsen <jwillemsen@remedy.nl>
 *
 * $Id$
 *
 * Wrapper facade for NDDS.
 */

#ifndef DDS4CCM_INCONSISTENTTOPICSTATUS_H
#define DDS4CCM_INCONSISTENTTOPICSTATUS_H

#include "dds4ccm/impl/dds4ccm_conf.h"

#if (CIAO_DDS4CCM_OPENDDS==1)
typedef ::DDS::InconsistentTopicStatus DDS_InconsistentTopicStatus;
#endif

#if (CIAO_DDS4CCM_NDDS==1)
inline void
operator<<= (::DDS::InconsistentTopicStatus &ddsstatus, const ::DDS_InconsistentTopicStatus & status)
{
  ddsstatus.total_count = status.total_count;
  ddsstatus.total_count_change = status.total_count_change;
}

inline void
operator<<= (::DDS_InconsistentTopicStatus &ddsstatus, const ::DDS::InconsistentTopicStatus & status)
{
  ddsstatus.total_count = status.total_count;
  ddsstatus.total_count_change = status.total_count_change;
}

#endif

#endif /* DDS4CCM_INCONSISTENTTOPICSTATUS_H */
