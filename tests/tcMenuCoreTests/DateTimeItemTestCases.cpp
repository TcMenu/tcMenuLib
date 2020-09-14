
#include <AUnit.h>
#include <RuntimeMenuItem.h>
#include <RemoteMenuItem.h>
#include <RemoteAuthentication.h>
#include "fixtures_extern.h"
#include <tcUtil.h>

void printMenuItem(MenuItem* menuItem);


RENDERING_CALLBACK_NAME_INVOKE(timeMenuItemTestCb, timeItemRenderFn, "Time", 103, NULL)
RENDERING_CALLBACK_NAME_INVOKE(dateFormattedTestCb, dateItemRenderFn, "Date", 999, NULL)

test(testTimeMenuItem12Hr) {
    TimeFormattedMenuItem timeItem24(timeMenuItemTestCb, 111, EDITMODE_TIME_12H);

    char sz[20];
    timeItem24.setTime(TimeStorage(12, 20, 30));
    timeItem24.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("12:20:30PM", sz);

    timeItem24.setTime(TimeStorage(0, 10, 30));
    timeItem24.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("12:10:30AM", sz);

    timeItem24.setTime(TimeStorage(11, 59, 30));
    timeItem24.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("11:59:30AM", sz);

    timeItem24.setTime(TimeStorage(23, 59, 30));
    timeItem24.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("11:59:30PM", sz);
}

test(testTimeMenuItem24Hr) {
    TimeFormattedMenuItem timeItem24(timeMenuItemTestCb, 111, EDITMODE_TIME_HUNDREDS_24H);

    char sz[20];
    timeItem24.setTime(TimeStorage(20, 39, 30, 93));
    timeItem24.copyNameToBuffer(sz, sizeof(sz));
    assertStringCaseEqual("Time", sz);
    timeItem24.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("20:39:30.93", sz);

    assertEqual(uint8_t(4), timeItem24.beginMultiEdit());
    assertEqual(23, timeItem24.nextPart());
    assertEqual(20, timeItem24.getPartValueAsInt());
    timeItem24.valueChanged(18);

    assertEqual(59, timeItem24.nextPart());
    assertEqual(39, timeItem24.getPartValueAsInt());
    timeItem24.valueChanged(30);

    timeItem24.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("18:[30]:30.93", sz);

    assertEqual(59, timeItem24.nextPart());
    assertEqual(30, timeItem24.getPartValueAsInt());

    assertEqual(99, timeItem24.nextPart());
    assertEqual(93, timeItem24.getPartValueAsInt());
    timeItem24.valueChanged(10);

    timeItem24.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("18:30:30.[10]", sz);
    timeItem24.stopMultiEdit();

    timeItem24.setTimeFromString("23:44:00.33");
    timeItem24.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("23:44:00.33", sz);

    timeItem24.setTimeFromString("8:32");
    timeItem24.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("08:32:00.00", sz);
}

test(dateFormattedMenuItem) {
    DateFormattedMenuItem dateItem(dateFormattedTestCb, 114, nullptr);

    char sz[25];
    dateItem.setDateFromString("2020/08/11");
    dateItem.copyNameToBuffer(sz, sizeof(sz));
    assertStringCaseEqual("Date", sz);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("11/08/2020", sz);

    assertEqual(uint8_t(3), dateItem.beginMultiEdit());
    assertEqual(31, dateItem.nextPart());
    assertEqual(11, dateItem.getPartValueAsInt());
    dateItem.valueChanged(10);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("[10]/08/2020", sz);

    assertEqual(12, dateItem.nextPart());
    assertEqual(8, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("10/[02]/2020", sz);

    assertEqual(9999, dateItem.nextPart());
    assertEqual(2020, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2018);

    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("10/02/[2018]", sz);
    dateItem.stopMultiEdit();

    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("10/02/2018", sz);

    DateFormattedMenuItem::setDateSeparator('-');
    DateFormattedMenuItem::setDateFormatStyle(DateFormattedMenuItem::MM_DD_YYYY);

    // now check MM-DD-YYYY
    dateItem.setDate(DateStorage(20, 11, 2019));
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("11-20-2019", sz);

    dateItem.beginMultiEdit();

    assertEqual(12, dateItem.nextPart());
    assertEqual(11, dateItem.getPartValueAsInt());
    dateItem.valueChanged(10);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("[10]-20-2019", sz);

    assertEqual(31, dateItem.nextPart());
    assertEqual(20, dateItem.getPartValueAsInt());
    dateItem.valueChanged(14);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("10-[14]-2019", sz);

    assertEqual(9999, dateItem.nextPart());
    assertEqual(2019, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2018);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("10-14-[2018]", sz);
    dateItem.stopMultiEdit();

    // lastly check YYYY-MM-DD

    DateFormattedMenuItem::setDateSeparator('*');
    DateFormattedMenuItem::setDateFormatStyle(DateFormattedMenuItem::YYYY_MM_DD);

    dateItem.setDate(DateStorage(20, 11, 2019));
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("2019*11*20", sz);

    dateItem.beginMultiEdit();

    assertEqual(9999, dateItem.nextPart());
    assertEqual(2019, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2018);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("[2018]*11*20", sz);

    assertEqual(12, dateItem.nextPart());
    assertEqual(11, dateItem.getPartValueAsInt());
    dateItem.valueChanged(3);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("2018*[03]*20", sz);

    assertEqual(31, dateItem.nextPart());
    assertEqual(20, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("2018*03*[02]", sz);
    dateItem.stopMultiEdit();
}

// visible for testing.
int daysForMonth(DateStorage& theDate);

test(testDateLeapYearAndMonthSizes) {

    auto theDate = DateStorage(3, 1, 1929);
    assertEqual(31, daysForMonth(theDate));

    // now do all the leap year test cases
    theDate.month = 2;
    assertEqual(28, daysForMonth(theDate)); // 1929 not a leap year
    theDate.year = 1900;
    assertEqual(28, daysForMonth(theDate)); // 1900 not a leap year. !(YR % 100)
    theDate.year = 1904;
    assertEqual(29, daysForMonth(theDate)); // 1904 is a leap year. YR % 4
    theDate.year = 1600;
    assertEqual(29, daysForMonth(theDate)); // 1600 is a leap year. YR % 400

    theDate.month = 3;
    assertEqual(31, daysForMonth(theDate));
    theDate.month = 4;
    assertEqual(30, daysForMonth(theDate));
    theDate.month = 5;
    assertEqual(31, daysForMonth(theDate));
    theDate.month = 6;
    assertEqual(30, daysForMonth(theDate));
    theDate.month = 7;
    assertEqual(31, daysForMonth(theDate));
    theDate.month = 8;
    assertEqual(31, daysForMonth(theDate));
    theDate.month = 9;
    assertEqual(30, daysForMonth(theDate));
    theDate.month = 10;
    assertEqual(31, daysForMonth(theDate));
    theDate.month = 11;
    assertEqual(30, daysForMonth(theDate));
    theDate.month = 12;
    assertEqual(31, daysForMonth(theDate));

}