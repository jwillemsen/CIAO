// $Id$

#ifndef __ROOTPANEL_H
#define __ROOTPANEL_H

#define QT_GENUINE_STR

#include "ace/Hash_Map_Manager.h"
#include "ace/Null_Mutex.h"
#include "MapView.h"
#include "DetailView.h"
#include <Qt/qmainwindow.h>
#include <Qt/qevent.h>
#include <Qt/q3popupmenu.h>

class NodeItem;
class QMouseEvent;

class RootPanel : public QMainWindow
{
  Q_OBJECT

typedef ACE_Hash_Map_Manager_Ex<ACE_UINT64,
                                NodeItem*,
                                ACE_Hash<ACE_UINT64>,
                                ACE_Equal_To<ACE_UINT64>,
                                ACE_Null_Mutex> NodeItemMap;

public:
  RootPanel(Q3Canvas&, QWidget *parent = 0, const char *name = 0);
  ~RootPanel();

public:
  void addUnit(NavUnit* unit);
  void updateUnit(NavUnit *unit);

protected:
  void clear();
  virtual void customEvent(QCustomEvent *e);

  Q3Canvas& canvas;
  MapView *navview;
  DetailView *details;

  Q3PopupMenu* options;
  NodeItemMap nodeMap;
};

#endif // __ROOTPANEL_H