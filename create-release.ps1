$path = "x64\Release\ArkGameTestingCommands"

New-Item -ItemType Directory -Force -Path $path
Copy-Item x64\Release\ArkGameTestingCommands.dll $path
Copy-Item x64\Release\ArkGameTestingCommands.pdb $path
Copy-Item Configs\Config.json $path
Copy-Item Configs\PluginInfo.json $path

Compress-Archive -Path $path -CompressionLevel Optimal -DestinationPath ArkGameTestingCommands-2.00.zip

Remove-Item $path -Force -Recurse -ErrorAction SilentlyContinue