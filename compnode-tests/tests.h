#include <QObject>
#include <QTest>

class CompNodeTestCase1 : public QObject
{
    Q_OBJECT

private slots:
    void Test1()
    {
        QVERIFY(1 != 2);
    }
};