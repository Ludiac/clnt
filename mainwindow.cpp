#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "LWItems.hpp"
#include "prompt_qdialog.hpp"
#include <QtNetwork/QtNetwork>
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  connect(&socket, &QTcpSocket::readyRead, this, &MainWindow::read);
  connect(&socket, &QTcpSocket::disconnected, this, &MainWindow::log_out);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::displayError(qint32 socketError, const QString &message) {
  switch (socketError) {
  case QAbstractSocket::HostNotFoundError:
    QMessageBox::information(this, tr("Blocking Fortune Client"),
                             tr("The host was not found. Please check the "
                                "host and port settings."));
    break;
  case QAbstractSocket::ConnectionRefusedError:
    QMessageBox::information(this, tr("Blocking Fortune Client"),
                             tr("The connection was refused by the peer. "
                                "Make sure the fortune server is running, "
                                "and check that the host name and port "
                                "settings are correct."));
    break;
  default:
    QMessageBox::information(
        this, tr("Blocking Fortune Client"),
        tr("The following error occurred: %1.").arg(message));
  }
}

void MainWindow::on_sendBtn_clicked() {
  if (rooms.current_room == 0)
    return;
  send(make_block(message_type::message, rooms.current_room,
                  ui->lineEdit->text()));
  ui->lineEdit->clear();
}

void MainWindow::read() {
  // QThreadPool::globalInstance()->start([&](){
  QDataStream in(&socket);
  in.setVersion(QDataStream::Qt_6_5);
  message_type type;
  while (socket.bytesAvailable()) {
    in.startTransaction();
    in >> type;
    qDebug() << "received"
             << static_cast<std::underlying_type_t<message_type>>(type)
             << "...";
    switch (type) {
    case message_type::ping: {
      //
    }; break;

    case message_type::sign_in: {
      bool successful;
      qint32 temp;
      in >> successful >> this_client_tag >> temp;
      in.commitTransaction();
      qDebug() << "..."
               << "succesful:" << successful << "tag:" << this_client_tag;
      if (successful) {
        this_client_id = temp;
        log_in();
      } else {
        QString error;
        if (temp == 1) {
          error = "user does not exist";
        } else if (temp == 2) {
          error = "this user is banned";
        } else if (temp == 3) {
          error = "invalid password";
        }
        QMessageBox::information(this, tr("signing in failed"), error);
      }
    }; break;

    case message_type::sign_up: {
      bool successful;
      in >> successful >> this_client_tag >> this_client_id;
      in.commitTransaction();
      qDebug() << "..."
               << "succesful:" << successful << "tag:" << this_client_tag
               << "id_in_db:" << this_client_id;
      if (successful)
        log_in();
      else
        QMessageBox::information(this, tr("signing up failed"),
                                 tr("invalid credentials"));
    }; break;

    case message_type::message: { // message
      qint32 room_id, message_id, sender_id;
      QString text;
      in >> room_id >> message_id >> sender_id >> text;
      in.commitTransaction();
      qDebug() << "..."
               << "room_id:" << room_id << "message_id:" << message_id
               << "sender_id:" << sender_id << "text:" << text;
      auto &&room_cache = rooms.get_room_cache(room_id);
      room_cache->append_message(message_id, sender_id, text);
      if (rooms.current_room == room_id) {
        ui->chatField->append(registered_clients.find(sender_id).value() +
                              ": " + text + "\n");
      }
    }; break;

    case message_type::room_messages: {
      qint32 room_id;
      QVector<QPair<qint32, QPair<qint32, QString>>> messages;
      in >> room_id >> messages;
      in.commitTransaction();
      qDebug() << "..."
               << "messages:" << messages;
      auto &&room_cache = rooms.get_room_cache(room_id);
      for (auto &&message : messages) {
        room_cache->append_message(message.first, message.second.first,
                                   message.second.second);
      }
    } break;

    case message_type::rooms_of_client: { // get open rooms
      QVector<QPair<qint32, QString>> roomss;
      in >> roomss;
      in.commitTransaction();
      qDebug() << "..."
               << "rooms:" << roomss;
      for (auto &&room : roomss) {
        if (rooms.add_room(room.first, room.second)) {
          send(make_block(message_type::room_messages, room.first),
               message_type::room_messages);
          send(make_block(message_type::room_members, room.first),
               message_type::room_members);
        }
      }
    }; break;

    case message_type::registered_clients: {
      QVector<QPair<qint32, QString>> clients;
      in >> clients;
      in.commitTransaction();
      qDebug() << "..."
               << "clients:" << clients;
      for (auto &&client : clients) {
        if (client.second != this_client_tag) {
          registered_clients.insert(client.first, client.second);
          new GeneralLWItem{client.first, client.second,
                            ui->registeredUsersList, 1001};
        } else {
          registered_clients.insert(client.first, "you");
          new GeneralLWItem{client.first, "you", ui->registeredUsersList, 1001};
        }
      }
    }; break;

    case message_type::room_members: { // room users
      // qDebug() << "getting room users";
      qint32 room_id;
      QVector<qint32> room_members;
      in >> room_id >> room_members;
      in.commitTransaction();
      qDebug() << "..."
               << "room_id:" << room_id << "room members:" << room_members;
      auto &&room_cache = rooms.get_room_cache(room_id);
      if (room_cache->publicc) {
        new GeneralLWItem{room_id, room_cache->name, ui->publicRoomsList, 1001};
        for (auto &&i : room_members) {
          room_cache->add_user(i);
        }
      } else {
        for (auto &&member_id : room_members) {
          if (member_id != this_client_id) {
            rooms.set_name(room_id, registered_clients.find(member_id).value());
            new PrivateRoomLWItem{room_id, member_id, room_cache->name,
                                  ui->privateRoomsList, 1002};
          }
          room_cache->add_user(member_id);
        }
      }
    } break;

    case message_type::invitation_in_public_room: {
      qint32 room_id, client_id;
      QString room_name;
      in >> room_id >> client_id >> room_name;
      in.commitTransaction();
      qDebug() << "..."
               << "room_id:" << room_id << ", client_id:" << client_id
               << ", room_name:" << room_name;
      rooms.add_room(room_id, room_name);
      // new GeneralLWItem{room_id, room_name, ui->publicRoomsList, 1001};
      QMessageBox::information(
          this, tr("Notification"),
          tr("You were invited and joined to public room: %1").arg(room_name));
      send(make_block(message_type::room_messages, room_id),
           message_type::room_messages);
      send(make_block(message_type::room_members, room_id),
           message_type::room_members);
    } break;

    case message_type::add_new_member_to_room: {
      qint32 room_id, client_id;
      in >> room_id >> client_id;
      in.commitTransaction();
      qDebug() << "..."
               << "room_id:" << room_id << ", client_id:" << client_id;
      auto &&a = rooms.get_room_cache(room_id);
      a->add_user(client_id);
      if (room_id == rooms.current_room)
        ui->roomMembersList->addItem(
            registered_clients.find(client_id).value());
    } break;

    case message_type::new_client_registered: {
      qint32 client_id;
      QString client_tag;
      in >> client_id >> client_tag;
      in.commitTransaction();
      qDebug() << "..."
               << "client_id:" << client_id << ", client_tag:" << client_tag;
      registered_clients.insert(client_id, client_tag);
      new GeneralLWItem{client_id, client_tag, ui->registeredUsersList, 1001};
    }; break;

    case message_type::ban: {
      in.commitTransaction();
      QMessageBox::information(this, tr("Notification"),
                               tr("This account was banned."));
      QApplication::exit();
    }; break;

    case message_type::force_disconnect: {
      in.commitTransaction();
      QMessageBox::information(this, tr("Notification"),
                               tr("Server denied connection"));
    }; break;

    default: {
      qDebug() << "bug occured! Invalid message_type received. terminating";
      std::terminate();
    }
    };
  }
  //});
}

void MainWindow::log_in() {
  send(make_block(message_type::registered_clients),
       message_type::registered_clients);
  send(make_block(message_type::rooms_of_client),
       message_type::rooms_of_client);
  ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::log_out() { QApplication::quit(); }

void MainWindow::send(QByteArray &&block) {
  qDebug() << "writing";
  socket.write(block);
}

void MainWindow::send(QByteArray &&block, message_type type) {
  qDebug() << "writing"
           << static_cast<std::underlying_type_t<message_type>>(type);
  socket.write(block);
}

void MainWindow::on_signinBtn_clicked() {
  send(make_block(message_type::sign_in, ui->loginLineEdit->text(),
                  ui->passwordLineEdit->text()));
}

void MainWindow::on_gotosignupBtn_clicked() {
  ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_gotosigninBtn_clicked() {
  ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_signupBtn_clicked() {
  if (ui->passwordLineEdit_signup->text() !=
      ui->repeatpasswordLineEdit_signup->text())
    QMessageBox::information(this, tr("chat"), tr("passwords don't match"));

  send(make_block(message_type::sign_up, ui->loginLineEdit_signup->text(),
                  ui->nameLineEdit_signup->text(),
                  ui->passwordLineEdit_signup->text()));
}

void MainWindow::on_privateRoomsList_currentItemChanged(
    QListWidgetItem *current, QListWidgetItem *previous) {
  auto &&room_id = current->data(1003).toInt();
  auto &&current_room = rooms.get_room_cache(room_id);
  rooms.current_room = room_id;
  ui->roomMembersList->clear();
  ui->chatField->clear();
  for (auto &&i : current_room->messages) {
    ui->chatField->append(registered_clients.find(i.sender_id).value() + ": " +
                          i.message + "\n");
  }
  for (auto &&i : current_room->members) {
    ui->roomMembersList->addItem(registered_clients.find(i).value());
  }
}

void MainWindow::on_publicRoomsList_currentItemChanged(
    QListWidgetItem *current, QListWidgetItem *previous) {
  auto &&room_id = current->data(1001).toInt();
  auto &&current_room = rooms.get_room_cache(room_id);
  rooms.current_room = room_id;
  ui->roomMembersList->clear();
  ui->chatField->clear();
  for (auto &&i : current_room->messages) {
    ui->chatField->append(registered_clients.find(i.sender_id).value() + ": " +
                          i.message + "\n");
  }
  for (auto &&i : current_room->members) {
    ui->roomMembersList->addItem(registered_clients.find(i).value());
  }
}

void MainWindow::on_startChattingBtn_clicked() {
  bool can_convert;
  auto &&client = ui->registeredUsersList->currentItem();
  if (client->text() == this_client_tag)
    return;
  for (int i = 0; i < ui->privateRoomsList->count(); ++i) {
    auto &&room = ui->privateRoomsList->item(i);
    if (room->text() == client->text()) {
      ui->privateRoomsList->setCurrentItem(room);
      return;
    }
  }
  send(make_block(message_type::create_private_room,
                  client->data(1001).toInt()));
}

void MainWindow::on_addUserToRoomBtn_clicked() {
  qint32 client_id =
      ui->registeredUsersList->currentItem()->data(1001).value<qint32>();
  qint32 room_id =
      ui->publicRoomsList->currentItem()->data(1001).value<qint32>();
  for (auto &&member : rooms.get_room_cache(room_id)->members) {
    if (member == client_id)
      return;
  }
  send(make_block(message_type::invitation_in_public_room, room_id, client_id));
}

void MainWindow::on_createPublicRoomBtn_clicked() {
  PromptQDialog dialog;
  dialog.exec();
  if (dialog.result()) {
    qDebug() << "prompt accepted"
             << "group name:" << dialog.group_name;
    send(make_block(message_type::create_public_room, dialog.group_name));
  } else {
    qDebug() << "prompt rejected";
  }
}

void MainWindow::on_connectBtn_clicked() {
  QString host{ui->serverHostLE->text()};
  qint32 port{ui->serverPortLE->text().toInt()};
  socket.connectToHost(host, port);
  if (!socket.waitForConnected(5000)) {
    qDebug() << "connection failure";
    QMessageBox::information(this, tr("eroor"), tr("something is wrong"));
    return;
  }
  ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_defaultServerBtn_clicked() {

  QHostAddress host{"188.242.64.99"};
  qint32 port{7999};
  socket.connectToHost(host, port);
  if (!socket.waitForConnected(20000)) {
    qDebug() << "connection failure";
    QMessageBox::information(this, tr("error"), tr("something is wrong"));
    return;
  }
  ui->stackedWidget->setCurrentIndex(1);
}
