/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtTest/QtTest>

#include <LiriAuroraUdev/Udev>
#include <LiriAuroraUdev/UdevEnumerate>

#include <umockdev.h>

using namespace QtUdev;

class TestUdev : public QObject
{
    Q_OBJECT
public:
    TestUdev(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

private:
    UMockdevTestbed *m_bed = nullptr;

private Q_SLOTS:
    void initTestCase()
    {
        m_bed = umockdev_testbed_new();
        QVERIFY(m_bed != nullptr);
        QString path = QFINDTESTDATA("test.umockdev");
        QVERIFY(!path.isEmpty());
        qInfo() << "Using:" << path;

        g_autoptr(GError) error = nullptr;
        umockdev_testbed_add_from_file(m_bed, path.toLatin1().constData(), &error);
        if (error)
            QFAIL(error->message);
        umockdev_testbed_enable(m_bed);

        QVERIFY(umockdev_in_mock_environment());
    }

    void cleanupTestCase()
    {
        if (m_bed)
            g_object_unref(m_bed);
    }

    void testConnection()
    {
        Udev *udev = new Udev;
        QVERIFY(udev->isValid());
        delete udev;
    }

    void testDevice()
    {
        Udev *udev = new Udev;
        QVERIFY(udev->isValid());

        UdevDevice *dev = udev->deviceFromFileName(QStringLiteral("/dev/sda"));
        QVERIFY(dev);
        QCOMPARE(dev->deviceNode(), QStringLiteral("/dev/sda"));
        QCOMPARE(dev->name(), QStringLiteral("sda"));
        QVERIFY(dev->hasProperty(QStringLiteral("ID_SERIAL")));
        QCOMPARE(dev->property(QStringLiteral("ID_SERIAL")), QStringLiteral("TOSHIBA_MQ01ABD100_Y49DSZAOS"));
        QCOMPARE(dev->property(QStringLiteral("ID_MODEL")), QStringLiteral("TOSHIBA_MQ01ABD100"));

        delete dev;
        delete udev;
    }

    void testEnumerate()
    {
        Udev *udev = new Udev;
        QVERIFY(udev->isValid());

        UdevEnumerate *enumerate = new UdevEnumerate(UdevDevice::InputDevice_Mask, udev);
        QList<UdevDevice *> devices = enumerate->scan();
        QVERIFY(devices.size() > 0);

        delete enumerate;
        delete udev;
    }
};

QTEST_MAIN(TestUdev)

#include "tst_udev.moc"
