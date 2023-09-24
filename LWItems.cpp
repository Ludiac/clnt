#include "LWItems.hpp"

GeneralLWItem::GeneralLWItem(qint32 id, QString text, QListWidget *parent,
                             int type)
    : id{id}, QListWidgetItem{text, parent, type} {}

PrivateRoomLWItem::PrivateRoomLWItem(qint32 room_id, qint32 client_id,
                                     QString text, QListWidget *parent,
                                     int type)
    : room_id{room_id}, client_id{client_id},
      QListWidgetItem{text, parent, type} {}

QVariant GeneralLWItem::data(int role) const {
  if (role == 1001) {
    return id;
  }
  return QListWidgetItem::data(role);
}

QVariant PrivateRoomLWItem::data(int role) const {
  if (role == 1002) {
    return client_id;
  } else if (role == 1003) {
    return room_id;
  }
  return QListWidgetItem::data(role);
}
