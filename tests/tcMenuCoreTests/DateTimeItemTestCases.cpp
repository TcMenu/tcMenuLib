
#include <testing/SimpleTest.h>
#include <RuntimeMenuItem.h>
#include <RemoteMenuItem.h>
#include <RemoteAuthentication.h>
#include "fixtures_extern.h"
#include <tcUtil.h>

void printMenuItem(MenuItem* menuItem);


RENDERING_CALLBACK_NAME_INVOKE(timeMenuItemTestCb, timeItemRenderFn, "Time", 103, NULL)
RENDERING_CALLBACK_NAME_INVOKE(dateFormattedTestCb, dateItemRenderFn, "Date", 999, NULL)

bool setTimeAndCompareResult(TimeFormattedMenuItem& item, int hr, int min, int sec, int hundreds, const char *expected) {
    char sz[20];
    item.setTime(TimeStorage(hr, min, sec, hundreds));
    item.copyValue(sz, sizeof sz);
    bool success = strncmp(expected, sz, sizeof sz) == 0;
    if(!success) {
        serdebugF4("Failed exp=", expected, ", act=", sz);
    }
    return success;
}

test(testTimeMenuItem12Hr) {
    TimeFormattedMenuItem timeItem12(timeMenuItemTestCb, 111, EDITMODE_TIME_12H);
    assertTrue(setTimeAndCompareResult(timeItem12, 12, 20, 30, 0, "12:20:30PM"));
    assertTrue(setTimeAndCompareResult(timeItem12, 12, 20, 30, 0, "12:20:30PM"));
    assertTrue(setTimeAndCompareResult(timeItem12, 0, 10, 30, 0, "12:10:30AM"));
    assertTrue(setTimeAndCompareResult(timeItem12, 11, 59, 30, 0, "11:59:30AM"));
    assertTrue(setTimeAndCompareResult(timeItem12, 23, 59, 30, 0, "11:59:30PM"));

    TimeFormattedMenuItem timeItem12HHMM(timeMenuItemTestCb, 111, EDITMODE_TIME_12H_HHMM);
    assertTrue(setTimeAndCompareResult(timeItem12HHMM, 12, 20, 30, 0, "12:20PM"));
    assertTrue(setTimeAndCompareResult(timeItem12HHMM, 12, 20, 30, 0, "12:20PM"));
    assertTrue(setTimeAndCompareResult(timeItem12HHMM, 23, 59, 30, 0, "11:59PM"));
}

test(testTimeMenuItem24Hr) {
    TimeFormattedMenuItem timeItem12(timeMenuItemTestCb, 111, EDITMODE_TIME_24H);
    assertTrue(setTimeAndCompareResult(timeItem12, 12, 20, 30, 0, "12:20:30"));
    assertTrue(setTimeAndCompareResult(timeItem12, 0, 10, 59, 0, "00:10:59"));
    assertTrue(setTimeAndCompareResult(timeItem12, 17, 59, 30, 0, "17:59:30"));
    assertTrue(setTimeAndCompareResult(timeItem12, 23, 59, 30, 0, "23:59:30"));

    TimeFormattedMenuItem timeItem12HHMM(timeMenuItemTestCb, 111, EDITMODE_TIME_24H_HHMM);
    assertTrue(setTimeAndCompareResult(timeItem12HHMM, 0, 20, 30, 0, "00:20"));
    assertTrue(setTimeAndCompareResult(timeItem12HHMM, 12, 20, 30, 0, "12:20"));
    assertTrue(setTimeAndCompareResult(timeItem12HHMM, 23, 59, 30, 0, "23:59"));
}

test(testTimeMenuItemDuration) {
    TimeFormattedMenuItem timeItemDurationSec(timeMenuItemTestCb, 111, EDITMODE_TIME_DURATION_SECONDS);
    assertTrue(setTimeAndCompareResult(timeItemDurationSec, 0, 20, 30, 0, "20:30"));
    assertTrue(setTimeAndCompareResult(timeItemDurationSec, 0, 0, 59, 11, "00:59"));
    assertTrue(setTimeAndCompareResult(timeItemDurationSec, 1, 59, 30, 0, "01:59:30"));

    TimeFormattedMenuItem timeItemDurationHundreds(timeMenuItemTestCb, 111, EDITMODE_TIME_DURATION_HUNDREDS);
    assertTrue(setTimeAndCompareResult(timeItemDurationHundreds, 0, 20, 30, 9, "20:30.09"));
    assertTrue(setTimeAndCompareResult(timeItemDurationHundreds, 0, 0, 59, 99, "00:59.99"));
    assertTrue(setTimeAndCompareResult(timeItemDurationHundreds, 1, 59, 30, 1, "01:59:30.01"));
}

test(testTimeMenuItem24HrEditing) {
    TimeFormattedMenuItem timeItem24(timeMenuItemTestCb, 111, EDITMODE_TIME_HUNDREDS_24H);

    char sz[20];
    timeItem24.copyNameToBuffer(sz, sizeof(sz));
    assertStringEquals("Time", sz);

    timeItem24.setTime(TimeStorage(20, 39, 30, 93));
    assertStringEquals("Time", sz);
    timeItem24.copyValue(sz, sizeof(sz));
    assertStringEquals("20:39:30.93", sz);

    assertEquals(uint8_t(4), timeItem24.beginMultiEdit());
    assertEquals(23, timeItem24.nextPart());
    assertEquals(20, timeItem24.getPartValueAsInt());
    timeItem24.valueChanged(18);

    assertEquals(59, timeItem24.nextPart());
    assertEquals(39, timeItem24.getPartValueAsInt());
    timeItem24.valueChanged(30);

    timeItem24.copyValue(sz, sizeof(sz));
    assertStringEquals("18:30:30.93", sz);
    assertTrue(checkEditorHints(3, 5, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(59, timeItem24.nextPart());
    assertEquals(30, timeItem24.getPartValueAsInt());

    assertEquals(99, timeItem24.nextPart());
    assertEquals(93, timeItem24.getPartValueAsInt());
    timeItem24.valueChanged(10);

    timeItem24.copyValue(sz, sizeof(sz));
    assertStringEquals("18:30:30.10", sz);
    assertTrue(checkEditorHints(9, 11, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    timeItem24.stopMultiEdit();

    timeItem24.setTimeFromString("23:44:00.33");
    timeItem24.copyValue(sz, sizeof(sz));
    assertStringEquals("23:44:00.33", sz);

    timeItem24.setTimeFromString("8:32");
    timeItem24.copyValue(sz, sizeof(sz));
    assertStringEquals("08:32:00.00", sz);
}

test(dateFormattedMenuItem) {
    DateFormattedMenuItem dateItem(dateFormattedTestCb, 114, nullptr);

    char sz[25];
    dateItem.setDateFromString("2020/08/11");
    dateItem.copyNameToBuffer(sz, sizeof(sz));
    assertStringEquals("Date", sz);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringEquals("11/08/2020", sz);

    assertEquals(uint8_t(3), dateItem.beginMultiEdit());
    assertEquals(30, dateItem.nextPart());
    assertEquals(10, dateItem.getPartValueAsInt());
    dateItem.valueChanged(9);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringEquals("10/08/2020", sz);
    assertTrue(checkEditorHints(0, 2, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(11, dateItem.nextPart());
    assertEquals(7, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringEquals("10/03/2020", sz);
    assertTrue(checkEditorHints(3, 5, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(9999, dateItem.nextPart());
    assertEquals(2020, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2018);

    dateItem.copyValue(sz, sizeof(sz));
    assertStringEquals("10/03/2018", sz);
    assertTrue(checkEditorHints(6, 10, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    dateItem.stopMultiEdit();

    dateItem.copyValue(sz, sizeof(sz));
    assertStringEquals("10/03/2018", sz);

    DateFormattedMenuItem::setDateSeparator('-');
    DateFormattedMenuItem::setDateFormatStyle(DateFormattedMenuItem::MM_DD_YYYY);

    // now check MM-DD-YYYY
    dateItem.setDate(DateStorage(20, 11, 2019));
    dateItem.copyValue(sz, sizeof(sz));
    assertStringEquals("11-20-2019", sz);

    dateItem.beginMultiEdit();

    assertEquals(11, dateItem.nextPart());
    assertEquals(10, dateItem.getPartValueAsInt());
    dateItem.valueChanged(9);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringEquals("10-20-2019", sz);
    assertTrue(checkEditorHints(0, 2, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(30, dateItem.nextPart());
    assertEquals(19, dateItem.getPartValueAsInt());
    dateItem.valueChanged(13);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringEquals("10-14-2019", sz);
    assertTrue(checkEditorHints(3, 5, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(9999, dateItem.nextPart());
    assertEquals(2019, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2018);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringEquals("10-14-2018", sz);
    assertTrue(checkEditorHints(6, 10, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    dateItem.stopMultiEdit();

    // lastly check YYYY-MM-DD

    DateFormattedMenuItem::setDateSeparator('*');
    DateFormattedMenuItem::setDateFormatStyle(DateFormattedMenuItem::YYYY_MM_DD);

    dateItem.setDate(DateStorage(20, 11, 2019));
    dateItem.copyValue(sz, sizeof(sz));
    assertStringEquals("2019*11*20", sz);

    dateItem.beginMultiEdit();

    assertEquals(9999, dateItem.nextPart());
    assertEquals(2019, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2018);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringEquals("2018*11*20", sz);
    assertTrue(checkEditorHints(0, 4, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(11, dateItem.nextPart());
    assertEquals(10, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringEquals("2018*03*20", sz);
    assertTrue(checkEditorHints(5, 7, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(30, dateItem.nextPart());
    assertEquals(19, dateItem.getPartValueAsInt());
    dateItem.valueChanged(1);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringEquals("2018*03*02", sz);
    assertTrue(checkEditorHints(8, 10, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    dateItem.stopMultiEdit();
}

// visible for testing.
int daysForMonth(DateStorage& theDate);

test(testDateLeapYearAndMonthSizes) {

    auto theDate = DateStorage(3, 1, 1929);
    assertEquals(31, daysForMonth(theDate));

    // now do all the leap year test cases
    theDate.month = 2;
    assertEquals(28, daysForMonth(theDate)); // 1929 not a leap year
    theDate.year = 1900;
    assertEquals(28, daysForMonth(theDate)); // 1900 not a leap year. !(YR % 100)
    theDate.year = 1904;
    assertEquals(29, daysForMonth(theDate)); // 1904 is a leap year. YR % 4
    theDate.year = 1600;
    assertEquals(29, daysForMonth(theDate)); // 1600 is a leap year. YR % 400

    theDate.month = 3;
    assertEquals(31, daysForMonth(theDate));
    theDate.month = 4;
    assertEquals(30, daysForMonth(theDate));
    theDate.month = 5;
    assertEquals(31, daysForMonth(theDate));
    theDate.month = 6;
    assertEquals(30, daysForMonth(theDate));
    theDate.month = 7;
    assertEquals(31, daysForMonth(theDate));
    theDate.month = 8;
    assertEquals(31, daysForMonth(theDate));
    theDate.month = 9;
    assertEquals(30, daysForMonth(theDate));
    theDate.month = 10;
    assertEquals(31, daysForMonth(theDate));
    theDate.month = 11;
    assertEquals(30, daysForMonth(theDate));
    theDate.month = 12;
    assertEquals(31, daysForMonth(theDate));

}