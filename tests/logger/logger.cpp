#include <gtest/gtest.h>

#include "zinctestsetup.hpp"
#include <opencmiss/zinc/zincconfigure.h>
#include <zinc/core.h>
#include <zinc/logger.h>
#include <zinc/region.h>
#include "zinctestsetupcpp.hpp"
#include <zinc/region.hpp>
#include <zinc/logger.hpp>

#include "test_resources.h"

TEST(cmzn_logger_messages, invalid_args)
{
	ZincTestSetup zinc;

	cmzn_logger_id logger = cmzn_context_get_logger(zinc.context);
	EXPECT_NE(static_cast<cmzn_logger *>(0), logger);

	EXPECT_EQ(0, cmzn_logger_get_number_of_messages(logger));

	cmzn_region_read_file(zinc.root_region, TestResources::getLocation(
		TestResources::REGION_INCORRECT_RESOURCE));

	cmzn_region_read_file(zinc.root_region, TestResources::getLocation(
		TestResources::REGION_INCORRECT_RESOURCE));

	int return_code = cmzn_logger_get_number_of_messages(logger);
	EXPECT_EQ(4, return_code);

	for (int i = 0; i < return_code; i++)
	{
		char *new_message = cmzn_logger_get_message_text_at_index(logger, i+1);
		EXPECT_NE((char *)0, new_message);
		cmzn_deallocate(new_message);
		EXPECT_EQ(CMZN_LOGGER_MESSAGE_TYPE_ERROR,
			cmzn_logger_get_message_type_at_index(logger, i+1));
	}

	EXPECT_EQ(CMZN_OK, cmzn_logger_set_maximum_number_of_messages(logger, 2));
	return_code = cmzn_logger_get_number_of_messages(logger);
	EXPECT_EQ(2, return_code);

	char *new_message = cmzn_logger_get_message_text_at_index(logger, 1);
	EXPECT_NE((char *)0, new_message);
	cmzn_deallocate(new_message);

	new_message = cmzn_logger_get_message_text_at_index(logger, 2);
	EXPECT_NE((char *)0, new_message);
	cmzn_deallocate(new_message);

	cmzn_logger_destroy(&logger);
}

static void custom_message_callback(cmzn_loggerevent_id event, void *user_data)
{
	cmzn_logger_change_flags flags = cmzn_loggerevent_get_change_flags(event);
	EXPECT_EQ(flags, CMZN_LOGGER_CHANGE_FLAG_NEW_MESSAGE);

	cmzn_logger_message_type type = cmzn_loggerevent_get_message_type(event);
	EXPECT_EQ(type, CMZN_LOGGER_MESSAGE_TYPE_ERROR);

	char *message = cmzn_loggerevent_get_message_text(event);
	EXPECT_NE((char *)0, message);
	cmzn_deallocate(message);

	cmzn_logger_id logger = cmzn_loggerevent_get_logger(event);
	EXPECT_NE(static_cast<cmzn_logger *>(0), logger);

	int return_code = cmzn_logger_get_number_of_messages(logger);
	EXPECT_EQ(1, return_code);

	EXPECT_EQ(CMZN_OK, cmzn_logger_remove_all_messages(logger));

	cmzn_logger_destroy(&logger);
}

TEST(cmzn_logger_messages_callback, invalid_args)
{
	ZincTestSetup zinc;

	cmzn_logger_id logger = cmzn_context_get_logger(zinc.context);
	EXPECT_NE(static_cast<cmzn_logger *>(0), logger);

	EXPECT_EQ(0, cmzn_logger_get_number_of_messages(logger));

	cmzn_loggernotifier_id loggerNotifier = cmzn_logger_create_loggernotifier(logger);
	EXPECT_NE(static_cast<cmzn_loggernotifier *>(0), loggerNotifier);

	EXPECT_EQ(CMZN_OK, cmzn_loggernotifier_set_callback(loggerNotifier, custom_message_callback, 0));

	cmzn_region_read_file(zinc.root_region, TestResources::getLocation(
		TestResources::REGION_INCORRECT_RESOURCE));

	/* callback flushes messages. */
	int return_code = cmzn_logger_get_number_of_messages(logger);
	EXPECT_EQ(0, return_code);

	EXPECT_EQ(CMZN_OK, cmzn_loggernotifier_clear_callback(loggerNotifier));

	cmzn_loggernotifier_destroy(&loggerNotifier);

	cmzn_logger_destroy(&logger);
}

class myLoggercallback : public Loggercallback
{
private:

	virtual void operator()(const Loggerevent &loggerevent)
	{
		EXPECT_EQ(Logger::CHANGE_FLAG_NEW_MESSAGE, loggerevent.getChangeFlags());
		Logger logger = loggerevent.getLogger();
		EXPECT_TRUE(logger.isValid());
		EXPECT_EQ(logger.getMessageTypeAtIndex(1), loggerevent.getMessageType());
		char *eventMessage = loggerevent.getMessageText();
		char *loggerMessage = logger.getMessageTextAtIndex(1);
		EXPECT_STREQ(eventMessage, loggerMessage);
		EXPECT_EQ(CMZN_OK, logger.removeAllMessages());
		cmzn_deallocate(eventMessage);
		cmzn_deallocate(loggerMessage);
	}

public:
	myLoggercallback() : Loggercallback()
	{
	}
};

TEST(ZincLogger, callback)
{
	ZincTestSetupCpp zinc;

	Logger logger = zinc.context.getLogger();
	Loggernotifier loggernotifier = logger.createLoggernotifier();
	EXPECT_TRUE(loggernotifier.isValid());
	myLoggercallback thisNotifier;
	loggernotifier.setCallback(thisNotifier);
	zinc.root_region.readFile(TestResources::getLocation(TestResources::REGION_INCORRECT_RESOURCE));
	EXPECT_EQ(CMZN_OK, loggernotifier.clearCallback());
}
