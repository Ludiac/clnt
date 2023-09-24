#pragma once

#include "qhash.h"
#include "qpointer.h"
#include "qstring.h"

struct Client {
  QString tag;
  qint32 id_in_db;
};

struct Message {
  qint32 id;
  qint32 sender_id;
  QString message;
};

Q_DECLARE_METATYPE(Message)

class RoomContent {
public:
  RoomContent(bool publicc);
  RoomContent(bool publicc, QString name);
  void add_user(qint32 client_id);
  void append_message(qint32 room_id, qint32 sender_id, QString text);

  QVector<Message> messages;
  QVector<qint32> members;
  bool publicc;
  QString name;
private:
  QString* get_cash();
};

class Rooms {
public:
  RoomContent *get_room_cache(qint32 id);
  void set_name(qint32 id, QString name);
  bool add_room(qint32 id, QString name);

  qint32 current_room{0};
private:
  QHash<qint32, RoomContent> room_caches;
};
