#include "mainwindow.h"
#include "ui_mainwindow.h"


/* 关闭设备，不要应用QT4的close*/
extern int close (int __fd);

/* 定时查看设备状态*/
void MainWindow::CheckMachineStatFunction(void)
{
    qDebug("%s", __FUNCTION__);

    /* 构造消息*/
    MachineInfo.CheckMachineStatMsg.MsgId  = PC_ARM_CHECKMACHINESTAT_REQ;
    MachineInfo.CheckMachineStatMsg.MsgLen = sizeof(MachineInfo)
                                                - sizeof(MachineInfo.CheckMachineStatMsg);

    /* 发送消息*/
    len = sendto(client_sockfd, &MachineInfo, sizeof(MachineInfo),
                 0, (struct sockaddr *)&client_addr, sin_size);
    if(len < 0) {
        qDebug() << "send message error!";
        return;
    }

    /* 接收系统信息*/
    len = recvfrom(client_sockfd, &MachineInfo, sizeof(MachineInfo), 0, 0, 0);
    if (len < 0) {
        qDebug() << "receive message error!";
        return;
    }

    /* 填充界面信息*/
    if(MachineInfo.CheckMachineStatMsg.MsgId == ARM_PC_CHECKMACHINESTAT_RSP) {
        if(MachineInfo.MachineStat == OK)
            ui->StatButton->setText(tr("正常"));
        else
            ui->StatButton->setText(tr("失败"));

        /* 显示温度*/
        ui->CurrnetTemperatureLineEdit->setText(QString::number(MachineInfo.TempData, 'g', 3));

        /* 显示电压*/
        ui->CurrentVoltageLineEdit->setText(QString::number(MachineInfo.Voltage, 'g', 3));

        /* 显示CPU频率*/
        ui->CpuFreqLineEdit->setText(QString::number(MachineInfo.CpuFreq, 10));

        /* 显示剩余内存*/
        ui->FreeMemLineEdit->setText(QString::number(MachineInfo.FreeMemSpace, 10));

        /* 显示剩余磁盘空间*/
        ui->FreeDiskSpaceLineEdit->setText(QString::number(MachineInfo.FreeDiskSpace, 10));
    }
}

/* 寄存器设置*/
void MainWindow::RegisterSettingFunction(void)
{
    qDebug("%s", __FUNCTION__);

    /* 通过界面信息构造RegsInfo*/
    RegsInfo.RegsMsg.MsgId = PC_ARM_RegisterControl;
    RegsInfo.RegsMsg.MsgLen = sizeof(RegsInfo) - sizeof(RegsInfo.RegsMsg);

    RegsInfo.Addr = ui->RegisterAddrLineEdit->displayText().toULong(&ok, 16);
    if(ok == false) {
        qDebug() << "convert regs addr error";
        return;
    }

    RegsInfo.Data = ui->RegisterDataLineEdit->displayText().toULong(&ok, 16);
    if(ok == false) {
        qDebug() << "convert regs data error";
        return;
    }

    if(ui->RegisterWriteCheckBox->isChecked() == true) {
        RegsInfo.flag   =   WRITE;
    } else if (ui->RegisterReadCheckBox->isChecked() == true) {
        RegsInfo.flag   =   READ;
    } else {
        qDebug() << "please select read or write action!";
        return;
    }

    /* 通过网络进行发送*/
    len = sendto(client_sockfd, &RegsInfo, sizeof(RegsInfo), 0,
                       (struct sockaddr *)&client_addr, sin_size);
    if(len < 0) {
        qDebug() << "send mesage to arm error!";
        return;
    }

    len = recvfrom(client_sockfd, &RegsInfo, sizeof(RegsInfo), 0, 0, 0);
    if(len < 0) {
        qDebug() << "recevice message error!";
        return;
    }

    if(RegsInfo.RegsMsg.MsgId == ARM_PC_RegisterControl_Rsp)
        if(RegsInfo.flag == READ)
            ui->RegisterDataLineEdit->setText(QString::number(RegsInfo.Data, 16));
}

/* EEPROM设置*/
void MainWindow::EepromSettingFunction(void)
{
    qDebug("%s", __FUNCTION__);

    /* 构造EepromInfo结构*/
    EepromInfo.ControlEepromMsg.MsgId = PC_ARM_EEPROM_Control;
    EepromInfo.ControlEepromMsg.MsgLen = sizeof(EepromInfo) -
                                            sizeof(EepromInfo.ControlEepromMsg);

    EepromInfo.addr =  ui->EepromAddrLineEdit->displayText().toULong(&ok, 16);
    if(ok == false) {
        qDebug() << "convert eeprom addr error!";
        return;
    }

    EepromInfo.data = ui->EepromDataLineEdit->displayText().toULong(&ok, 16);
    if(ok == false) {
        qDebug() << "convert eeprom data error!";
        return;
    }

    if(ui->EepromWriteCheckBox->isChecked() == true) {
        EepromInfo.flag = WRITE;
    } else if(ui->EepromReadCheckBox->isChecked() == true) {
        EepromInfo.flag = READ;
    } else
        qDebug() << "please select read or write action!";

    /* 发送消息给ARM*/
    len = sendto(client_sockfd, &EepromInfo, sizeof(EepromInfo), 0,
                            (struct sockaddr *)&client_addr, sin_size);
    if (len < 0) {
        qDebug("send message to ARM error!");
        return;
    }

    /* 接受消息*/
    len = recvfrom(client_sockfd, &EepromInfo, sizeof(EepromInfo),0, 0 ,0);
    if(len < 0) {
        qDebug("receive message from ARM error!");
        return;
    }

    if(EepromInfo.ControlEepromMsg.MsgId == ARM_PC_EEPROM_Control_Rsp) {
        if(EepromInfo.flag == READ) {
            ui->EepromDataLineEdit->setText(QString::number(EepromInfo.data, 16));
        }
    }
}

/* 发送GRPS短信息*/
void MainWindow::SendGprsMessageFunction(void)
{
    qDebug("%s", __FUNCTION__);

    /* 构造消息信息*/
    GprsMessageInfo.GprsMessageMsg.MsgId = PC_ARM_SENDGPRSMESSAGE_REQ;
    GprsMessageInfo.GprsMessageMsg.MsgLen = sizeof(GprsMessageInfo) -
                                                sizeof(GprsMessageInfo.GprsMessageMsg);

    strcpy(GprsMessageInfo.TelephoneNumber,
            ui->TelephoneNumberLineEdit->displayText().toLocal8Bit());
    //str.toLatin1().data()
    strcpy(GprsMessageInfo.Message,
            ui->MessageLineEdit->displayText().toLocal8Bit());

    qDebug("telephone = %s message = %s\n",
            GprsMessageInfo.TelephoneNumber, GprsMessageInfo.Message);

    /* 发送消息*/
    len = sendto(client_sockfd, &GprsMessageInfo, sizeof(GprsMessageInfo),
                               0, (struct sockaddr *)&client_addr, sin_size);
    if(len < 0) {
        qDebug("send gprs message to arm error!");
        return;
    }

    /* 接收是否发送成功*/
    len = recvfrom(client_sockfd, &GprsMessageInfo, sizeof(GprsMessageInfo), 0, 0, 0);
    if(len < 0) {
        qDebug("receive message from arm error!");
        return;
    }

    if (GprsMessageInfo.GprsMessageMsg.MsgId == ARM_PC_SENDGPRSMESSAGE_RSP) {
        if(GprsMessageInfo.Result == -1) {
            QMessageBox::question(this, tr("结果"),QString(tr("发送失败")));
        } else {
            QMessageBox::question(this, tr("结果"),QString(tr("发送成功")));
        }
    }
}

/* 发送zigbee控制命令*/
void MainWindow::SendZigbeeMessageFunction(void)
{
    qDebug("%s", __FUNCTION__);

    /* 构造消息*/
    ZigbeeMessageInfo.ZigbeeMessageMsg.MsgId = PC_ARM_SENDZIGBEEMESSAGE_REQ;
    ZigbeeMessageInfo.ZigbeeMessageMsg.MsgLen = sizeof(ZigbeeMessageInfo) -
                                                    sizeof(ZigbeeMessageInfo.ZigbeeMessageMsg);
    strcpy(ZigbeeMessageInfo.Message,
            ui->ZigbeeMessageLineEdit->displayText().toLocal8Bit());

    /* 发送消息*/
    len = sendto(client_sockfd, &ZigbeeMessageInfo, sizeof(ZigbeeMessageInfo),
                 0, (struct sockaddr *)&client_addr, sin_size);
    if(len < 0) {
        qDebug("send zigbee message to arm error!");
        return;
    }

    len = recvfrom(client_sockfd, &ZigbeeMessageInfo, sizeof(ZigbeeMessageInfo),
                   0, 0, 0);
    if (len < 0) {
        qDebug("receive message from arm error!");
        return;
    }

    /* 接收处理信息*/
    if (ZigbeeMessageInfo.ZigbeeMessageMsg.MsgId == ARM_PC_SENDZIGBEEMESSAGE_RSP) {
        if (ZigbeeMessageInfo.Result == -1) {
            QMessageBox::question(this, tr("结果"), QString(tr("发送失败")));
        } else {
            QMessageBox::question(this, tr("结果"), QString(tr("发送成功")));
        }
    }
}

/* 报警器控制*/
void MainWindow::BeepSettingFunction(void)
{
    qDebug("%s", __FUNCTION__);

    /* 构造消息*/
    BeepInfo.BeepControlMsg.MsgId = PC_ARM_BEEPCONTROL_REQ;
    BeepInfo.BeepControlMsg.MsgLen = sizeof(BeepInfo) -
                                        sizeof(BeepInfo.BeepControlMsg);

    if(ui->BeepOpenCheckBox->isChecked() == true) {
        BeepInfo.Flag = 1;
    }
    else if(ui->BeepCloseCheckBox->isChecked() == true) {
        BeepInfo.Flag = 0;
    } else {
        BeepInfo.Flag = -1; //无效
    }

    /* 发送消息*/
    len = sendto(client_sockfd, &BeepInfo, sizeof(BeepInfo),
                 0, (struct sockaddr *)&client_addr, sin_size);
    if (len < 0) {
        qDebug("send beep message to arm error!");
        return;
    }

    len = recvfrom(client_sockfd, &BeepInfo, sizeof(BeepInfo), 0, 0, 0);
    if (len < 0) {
        qDebug("receive message from arm error!");
        return;
    }

    if (BeepInfo.BeepControlMsg.MsgId == ARM_PC_BEEPCONTROL_RSP) {
        if (BeepInfo.Result == -1) {
            QMessageBox::question(this, tr("结果"),QString(tr("打开失败")));
        } else
            QMessageBox::question(this, tr("结果"),QString(tr("打开成功")));
    }
}

/* LED 控制*/
void MainWindow::LedSettingFunction(void)
{
    qDebug("%s", __FUNCTION__);

    /* 构造消息*/
    LedInfo.LedControlMsg.MsgId = PC_ARM_LEDCONTROL_REQ;
    LedInfo.LedControlMsg.MsgLen = ARM_PC_LEDCONTROL_RSP;

    if (ui->LedOpenCheckBox->isChecked() == true) {
        LedInfo.Flag = 1;
    } else if (ui->LedCloseCheckBox->isChecked() == true) {
        LedInfo.Flag = 0;
    } else {
        LedInfo.Flag = -1;
    }

    if(LedInfo.Flag == 1) {
        LedInfo.FlashTime = ui->LedFlashTimeLineEdit->displayText().toULong(&ok, 10);
        if (ok == false) {
            qDebug("convert led flash time error!");
            return;
        }
    }

    /* 发送消息 */
    len = sendto(client_sockfd, &LedInfo, sizeof(LedInfo),
                 0, (struct sockaddr *)&client_addr, sin_size);
    if (len < 0) {
        qDebug("send led message to arm error!");
        return;
    }

    len = recvfrom(client_sockfd, &LedInfo, sizeof(LedInfo), 0, 0, 0);
    if (len < 0) {
        qDebug("receive led message from arm error!");
        return;
    }

    if (LedInfo.LedControlMsg.MsgId == ARM_PC_LEDCONTROL_RSP) {
        if (LedInfo.Result == -1) {
            QMessageBox::question(this, tr("结果"),QString(tr("设置失败")));
        } else
            QMessageBox::question(this, tr("结果"),QString(tr("设置成功")));
    }
}

/* 设备自检查询*/
void MainWindow::HardWareTestSelfFunction(void)
{
    qDebug("%s", __FUNCTION__);

    /* 目前只是通过发送消息头来判断系统运行状态*/
    HardWareInfo.HardWareTestSelfMsg.MsgId = PC_ARM_HARDWARE_TESTSELF_REQ;
    HardWareInfo.HardWareTestSelfMsg.MsgLen = sizeof(HardWareInfo)
                                                - sizeof(HardWareInfo.HardWareTestSelfMsg);

    /*发送消息头 */
    len = sendto(client_sockfd, &HardWareInfo, sizeof(HardWareInfo),
                 0, (struct sockaddr *)&client_addr, sin_size);
    if (len < 0) {
        qDebug("send hardware test message to arm error!");
        return;
    }

    len = recvfrom(client_sockfd, &HardWareInfo, sizeof(HardWareInfo), 0, 0, 0);
    if (len < 0) {
        qDebug("receive message from arm error!");
        return;
    }

    if (HardWareInfo.HardWareTestSelfMsg.MsgId == ARM_PC_HARDWARE_TESTSELF_RSP) {
        ui->HardWareTestSelfLineEdit->setText(tr("正常"));
    }
}

/* 软件更新*/
/* 遗留更新进度？*/
void MainWindow::SoftWareUpdateFunction(void)
{
    int fd          = -1;
    int length      = 0;
    int xferlen     = 0;

    /* 选择更新的文件*/
    QString FileName = FileDialog->getOpenFileName(this);
    if (FileName.isEmpty()) {
        qDebug("please select a file!");
        return;
    }

    /* 获取文件属性*/
    File     = new QFile(FileName);
    FileInfo = new QFileInfo(FileName);
    processdlg->setRange(0, File->size());

    /* 打开文件*/
    fd = open(File->fileName().toLocal8Bit(), O_RDONLY);
    if(fd < 0) {
        qDebug() << "pen file error!";
        return;
    }
    strcpy(SW_Update.FileName, FileInfo->fileName().toLocal8Bit());

    /* 传输文件，每次传输5K*/
    while ((length = read(fd, SW_Update.buf, BUF_SIZE)) > 0) {
        /* 构造消息头*/
        SW_Update.SoftWareUpdateMsg.MsgId  = PC_ARM_SOFTWARE_UPDATE_REQ;
        SW_Update.SoftWareUpdateMsg.MsgLen = sizeof(SW_Update)
                                               - sizeof(SW_Update.SoftWareUpdateMsg);
        SW_Update.Len   =   length;

           /* 发送信息*/
        len = sendto(client_sockfd, &SW_Update, sizeof(SW_Update),
                     0, (struct sockaddr *)&client_addr, sin_size);
        if (len < 0) {
            qDebug() << "send software update message to arm error!";
            return;
        }

        /* 查看更新结果*/
        len = recvfrom(client_sockfd, &SW_Update, sizeof(SW_Update), 0, 0, 0);
        if (len < 0) {
            qDebug() << "receive message from arm error!";
               return;
        }

        /* 解析反馈结果*/
        if(SW_Update.SoftWareUpdateMsg.MsgId == ARM_PC_SOFTWARE_UPDATE_RSP) {
            if(SW_Update.Result == -1) {
                QMessageBox::question(this, tr("更新结果"),QString(tr("更新失败")));
                return;
            } else {
                /* 显示进度*/
                xferlen += length;
                processdlg->setValue(xferlen);
                if (xferlen == File->size())
                    QMessageBox::question(this, tr("更新结果"),QString(tr("更新成功")));
            }
        }
        bzero(SW_Update.buf, length);
    }
    ::close(fd);
}

/* 更新系统界面时间*/
void MainWindow::UpdateWallTimeFunction(void)
{
    ui->CurrentTimeLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

/* 校准时间*/
void MainWindow::TimeAdjustFunction(void)
{
    QDate   date;
    QTime   time;

    /* 获取日期信息*/
    date    =   ui->dateTimeEdit->date();
    time    =   ui->dateTimeEdit->time();

    /* 构造消息*/
    TimeInfo.TimeAdjustMsg.MsgId    = PC_ARM_TIME_ADJUST_REQ;
    TimeInfo.TimeAdjustMsg.MsgLen   = sizeof(TimeInfo)
                                        - sizeof(TimeInfo.TimeAdjustMsg);
    /* 解析年月日时分秒*/
    TimeInfo.year        =  date.year();
    TimeInfo.month       =  date.month();
    TimeInfo.day         =  date.day();
    TimeInfo.hour        =  time.hour();
    TimeInfo.min         =  time.minute();
    TimeInfo.sec         =  time.second();

    len  = sendto(client_sockfd, &TimeInfo, sizeof(TimeInfo),
                  0, (struct sockaddr *)&client_addr, sin_size);
    if (len < 0) {
        qDebug("send time adjust message error!");
        return;
    }

    len = recvfrom(client_sockfd, &TimeInfo, sizeof(TimeInfo), 0, 0, 0);
    if (len < 0) {
        qDebug("receive time adjust result error!");
        return;
    }

    if (TimeInfo.TimeAdjustMsg.MsgId == ARM_PC_TIME_ADJUST_RSP) {
        if (TimeInfo.Result == -1)
            QMessageBox::information(this, tr("设置结果"), QString(tr("设置失败")));
        else
            QMessageBox::information(this, tr("设置结果"), QString(tr("设置成功")));
    }
}
