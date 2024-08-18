#include <unity.h>
#include <tcMenu.h>
#include <BaseDialog.h>
#include <RemoteConnector.h>

//
// test implementation of the dialog, just counts up the number of render calls.
// and gives access to the button text.
//
class DialogTestImpl : public BaseDialog {
private:
	int renderCount;
	int valueWhenRenderered;
	char szTemp[20];
public:
	DialogTestImpl(bool compressed) {
		bitWrite(flags, DLG_FLAG_SMALLDISPLAY, compressed);
		reset();
	}

	void reset() {
		renderCount = 0;
		valueWhenRenderered = -1;
	}

	void internalRender(int currentValue) override {
		renderCount++;
		valueWhenRenderered = currentValue;
	}

	int getValueWhenRenderered() { return valueWhenRenderered; }
	int getRenderCount() { return renderCount; }
	const char* getButtonText(int btn) {
	    copyButtonText(szTemp, btn, valueWhenRenderered, lastBtnVal == btn);
	    return szTemp;
	}
};

const char myHeader[] = "Hello";
ButtonType lastPressedBtn = BTNTYPE_NONE;
void* copyOfUserData;

void dialogTestCallback(ButtonType buttonPressed, void* yourData) {
	lastPressedBtn = buttonPressed;
	copyOfUserData = yourData;
}

void testBaseDialogInfo() {
    switches.resetAllSwitches();
	NoRenderer noRenderer;
	DialogTestImpl dialog(true);
	dialog.setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
	dialog.show(myHeader, false, NULL); // cannot send remotely.
	dialog.copyIntoBuffer("Blah blah");

	// should be inuse when shown
	TEST_ASSERT_TRUE(dialog.isInUse());

	// when not a remote dialog, should never be remotely sent
	TEST_ASSERT_FALSE(dialog.isRemoteUpdateNeeded(0));
	TEST_ASSERT_FALSE(dialog.isRemoteUpdateNeeded(1));
	TEST_ASSERT_FALSE(dialog.isRemoteUpdateNeeded(2));
	dialog.setRemoteUpdateNeeded(1, true);
	TEST_ASSERT_FALSE(dialog.isRemoteUpdateNeeded(1));

	// now try the rendering
	dialog.reset();
	dialog.dialogRendering(0, false);
	TEST_ASSERT_EQUAL(1, dialog.getRenderCount());
	TEST_ASSERT_EQUAL(0, dialog.getValueWhenRenderered());
	TEST_ASSERT_EQUAL_STRING("Blah blah", MenuRenderer::getInstance()->getBuffer());
    TEST_ASSERT_EQUAL_STRING("CLOSE", dialog.getButtonText(1));

	// and dismiss it by the button being clicked.
	dialog.dialogRendering(0, true);
	TEST_ASSERT_FALSE(dialog.isInUse());
}

void testBaseDialogQuestion() {
	NoRenderer noRenderer;
	DialogTestImpl dialog(false);
	dialog.setUserData((void*)myHeader);
	dialog.setButtons(BTNTYPE_OK, BTNTYPE_CANCEL);
	dialog.show(myHeader, true, dialogTestCallback); // can send remote, has callback.
	dialog.copyIntoBuffer("buffer text");
	TEST_ASSERT_TRUE(dialog.isInUse());

	// check that the buffer is copied and it will need a remote send.
	TEST_ASSERT_EQUAL_STRING("buffer text         ", MenuRenderer::getInstance()->getBuffer());
	TEST_ASSERT_TRUE(dialog.isRemoteUpdateNeeded(0));
	TEST_ASSERT_TRUE(dialog.isRemoteUpdateNeeded(1));
	TEST_ASSERT_TRUE(dialog.isRemoteUpdateNeeded(2));

	// clear only one remote send and check.
	dialog.setRemoteUpdateNeeded(1, false);
	TEST_ASSERT_TRUE(dialog.isRemoteUpdateNeeded(0));
	TEST_ASSERT_FALSE(dialog.isRemoteUpdateNeeded(1));
	TEST_ASSERT_TRUE(dialog.isRemoteUpdateNeeded(2));

	// reset the text and ensure it requires remote send again
	dialog.copyIntoBuffer("newText");
	TEST_ASSERT_TRUE(dialog.isRemoteUpdateNeeded(1));

	// now simualte render
	dialog.reset();
	dialog.dialogRendering(0, false);
	TEST_ASSERT_EQUAL(1, dialog.getRenderCount());
	TEST_ASSERT_EQUAL(0, dialog.getValueWhenRenderered());
	TEST_ASSERT_EQUAL_STRING("newText             ", MenuRenderer::getInstance()->getBuffer());
    TEST_ASSERT_EQUAL_STRING("OK", dialog.getButtonText(0));
    TEST_ASSERT_EQUAL_STRING("cancel", dialog.getButtonText(1));

	// and again, this time choose 2nd button.
	dialog.dialogRendering(1, false);
	TEST_ASSERT_EQUAL(2, dialog.getRenderCount());
    TEST_ASSERT_EQUAL_STRING("ok", dialog.getButtonText(0));
    TEST_ASSERT_EQUAL_STRING("CANCEL", dialog.getButtonText(1));

	// and now simulate the remote cancelling
	dialog.remoteAction(BTNTYPE_CANCEL);
	TEST_ASSERT_EQUAL(BTNTYPE_CANCEL, lastPressedBtn);
	TEST_ASSERT_EQUAL(myHeader, copyOfUserData);

	// it should not be in use after this.
	TEST_ASSERT_FALSE(dialog.isInUse());
}

