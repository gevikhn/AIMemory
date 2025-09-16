@echo off
echo AIMemory CLI Basic Usage Examples
echo ==================================
echo.

echo 1. Show help information
memctl --help
echo.

echo 2. Show version
memctl version
echo.

echo 3. Configuration management
echo Setting a configuration value...
memctl config set log_level DEBUG
echo.

echo Getting a configuration value...
memctl config get log_level
echo.

echo 4. Basic commands (placeholders - will be implemented)
echo Index management...
memctl index --help
echo.

echo Graph operations...
memctl graph --help
echo.

echo Memory recall...
memctl recall --help
echo.

echo 5. View metrics
memctl metrics --help
echo.

echo Basic usage examples completed!
pause