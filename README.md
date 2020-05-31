# Socket-Over-Tor
Make connections trough the TOR network
#### How to use

```c++
//Create class object
TorSocket tor;

//Conect
tor.Connect("www.3stas.c0m0-l0k1ta.com", "80");

//send data
tor.SendData(Buffer, BufferSize);

//read data
tor.ReadData(OutputBuffer);
//default size of output is allocated to 2048, if response is bigger more space is allocated automatically

//close connection
tor.CloseSocket();
```
