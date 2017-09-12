cmd.exe /C "start test_tx_pipe.bat 20"

cmd.exe /C "start test_rx_pipe.bat 4"
cmd.exe /C "start test_rx_pipe.bat 1"

timeout /T 3

cmd.exe /C "start test_rx_pipe.bat 2"
