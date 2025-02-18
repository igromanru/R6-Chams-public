# R6 Chams - Public source by Igromanru

## How to use without BattlEye 
1. Use latest Visual Studio to compile the source code into a DLL
2. Open Ubisoft Connect->Library->"..."->Properties
3. Under "Add launch arguments" add `/Belaunch -be` and press **Save** 
4. Start tha game normally (NOT the Vulkan version)
5. Use any injector to inject **R6-Chams-public.dll** into the process `RainbowSix_DX11.exe`

The *Debug* build should definitely be only used without BE and it will provide useful information if something doesn't work.

## Disclaimer
This source code is intended for educational purposes only, to demonstration how Chameleon Models with a "visual check" can be achieved with a more or less clean code.  
I don't use this code while AntiCheat is running and can't guarantee that your account will not be penalized if you use it.    
Use it at your own risk!  

## Possible detection vectors
- WinApi calls
- Detour hook
- Thread creation
  
<sub>That said, my code doesn't do anything what would be more risky than other public sources. If you have a way to use other public Chams without getting banned, my chams are as safe as them.  
However, I still recommend to make your own project and to avoid as many detection vectors as possible</sub>

## Demo
![visible](/pictures/visible.jpg)
![half_visible](/pictures/half_visible.jpg)
![invisible](/pictures/invisible.jpg)