
#include <log4cpp/Category.hh>
extern log4cpp::Category& logger;

#define DEBUG(...)  logger.debug(__VA_ARGS__)
#define INFO(...)   logger.info(__VA_ARGS__)
#define ERROR(...)  logger.info(__VA_ARGS__)
#define LOG(...)    logger.notice(__VA_ARGS__)

#ifdef LOGGER_COMPILE

#include <log4cpp/Appender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PatternLayout.hh>


log4cpp::Category& init_logger()
{
    log4cpp::Category& logger = log4cpp::Category::getRoot();
    logger.setPriority(log4cpp::Priority::DEBUG);

    log4cpp::Appender *console_appender = new log4cpp::OstreamAppender(
            "console", &std::cout);
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d{%H:%M:%S:%m} [%p] %m%n");
    console_appender->setLayout(layout);

    logger.addAppender(console_appender);

    return logger;
}

log4cpp::Category& logger = init_logger();

#endif // LOGGER_COMPILE
