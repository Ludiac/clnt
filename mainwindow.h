#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "network_thread.hpp"
#include "qlistwidget.h"
#include "qmutex.h"
#include <QMainWindow>
#include <QDataStream>
#include <QtNetwork/QTcpSocket>

enum class message_type : qint32 {
  ping = 1,
  sign_in,
  sign_up,
  message,
  room_messages,
  rooms_of_client,
  registered_clients,
  room_members,
  create_private_room,
  create_public_room,
  invitation_in_public_room,
  add_new_member_to_room,
  new_client_registered,
  ban,
  force_disconnect
};

template<typename... Targs>
QByteArray make_block(Targs... targs) {
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_6_5);
  (out << ... << targs);
  return block;
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

signals:

private slots:
  void read();
  void displayError(qint32 socketError, const QString &message);
  void on_sendBtn_clicked();
  void on_signinBtn_clicked();
  void on_gotosignupBtn_clicked();
  void on_gotosigninBtn_clicked();
  void on_signupBtn_clicked();
  void on_privateRoomsList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
  void on_startChattingBtn_clicked();
  void on_addUserToRoomBtn_clicked();
  void on_createPublicRoomBtn_clicked();
  void on_publicRoomsList_currentItemChanged(QListWidgetItem *current,
                                             QListWidgetItem *previous);

  void log_out();

  void on_connectBtn_clicked();

  void on_defaultServerBtn_clicked();

private:
  void send(QByteArray && block);
  void send(QByteArray &&block, message_type type);
  void log_in();

private:
  Ui::MainWindow *ui;
  QTcpSocket socket;
  Rooms rooms;
  QMap<qint32, QString> registered_clients;
  QString this_client_tag;
  qint32 this_client_id;
  QString server_host;
  qint32 server_port;
};

#endif // MAINWINDOW_H
