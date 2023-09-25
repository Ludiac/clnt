#include "network_thread.hpp"
#include "qdebug.h"

RoomContent::RoomContent(bool publicc) : publicc{publicc} {}
RoomContent::RoomContent(bool publicc, QString name) : publicc{publicc}, name{name} {}

void RoomContent::add_user(qint32 client_id) {
  members.append(client_id);
};

void RoomContent::append_message(qint32 room_id, qint32 sender_id, QString text) {
  messages.append({room_id, sender_id, text});
};

RoomContent *Rooms::get_room_cache(qint32 id)
{
  auto&& a = room_caches.find(id);
  if (a != room_caches.end()) {
    return &a.value();
  } else {
    qDebug() << "bug occured. didnt find room with id" << id;
    std::terminate();
  }
}

bool Rooms::add_room(qint32 id, QString name)
{
  if (room_caches.find(id) != room_caches.end()) return false;
  if (name != "") {
    room_caches.emplace(id, true, name);
  } else {
    room_caches.emplace(id, false);
  }
  return true;
}

void Rooms::set_name(qint32 id, QString name)
{
  get_room_cache(id)->name = name;
}
