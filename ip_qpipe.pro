TEMPLATE = subdirs

include(build.pri)

SUBDIRS =                \
            src          \
            test_rx_pipe \
            test_tx_pipe

src.subdir           = src
test_rx_pipe.subdir  = test/test_rx_pipe
test_tx_pipe.subdir  = test/test_tx_pipe

test_rx_pipe.depends = src
test_tx_pipe.depends = src

CONFIG += ordered

