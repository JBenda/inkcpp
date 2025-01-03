import inkcpp_py as ink
import pytest

class Observer:
    def __init__(self):
        self.cnt = 0
    def __call__(self, new, old):
        self.cnt += 1
        if self.cnt == 1:
            assert new.type == ink.Value.Int32 and str(new) == '1'
            assert old is None
        else:
            assert new.type == ink.Value.Int32 and str(new) == '5'
            assert old.type == ink.Value.Int32 and str(old) == '1'
            

class TestObserver:
    def test_observer(self, assets, generate):
        [story, store, runner] = generate(assets['ObserverStory'])

        obs = Observer()
        store.observe('var1', obs)

        assert runner.getline() == "hello line 1 1 hello line 2 5 test line 3 5\n"
        assert obs.cnt == 2

        
