import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]

class InputLifecycleRegressionTests(unittest.TestCase):
    def test_runtime_actions_exist_before_bindings(self):
        source = (ROOT / 'Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp').read_text(encoding='utf-8')
        start = source.index('void AOpenDrivePlayerController::SetupInputComponent()')
        body = source[start:]
        self.assertIn('BuildRuntimeInputMap();', body)
        self.assertLess(body.index('BuildRuntimeInputMap();'), body.index('BindAction('))

    def test_begin_play_registers_mapping_context(self):
        source = (ROOT / 'Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp').read_text(encoding='utf-8')
        start = source.index('void AOpenDrivePlayerController::BeginPlay()')
        body = source[start:source.index('void AOpenDrivePlayerController::BuildRuntimeInputMap()')]
        self.assertIn('RegisterRuntimeInputMap();', body)

if __name__ == '__main__':
    unittest.main()
