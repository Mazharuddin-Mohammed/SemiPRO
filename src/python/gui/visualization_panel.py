from PySide6.QtWidgets import QWidget
from ..cython.renderer import PyVulkanRenderer

class VisualizationPanel(QWidget):
    def __init__(self, width: int, height: int):
        super().__init__()
        self.renderer = PyVulkanRenderer(width, height)
        self.renderer.initialize()
        self.show_temperature_overlay = False
        self.show_reliability = False

    def update_visualization(self, wafer):
        self.renderer.render(wafer, self.show_temperature_overlay, self.show_reliability)
        self.wafer = wafer  # Store for re-rendering

    def set_temperature_overlay(self, state: bool):
        self.show_temperature_overlay = state
        if hasattr(self, 'wafer'):
            self.update_visualization(self.wafer)

    def set_reliability_view(self, state: bool):
        self.show_reliability = state
        if hasattr(self, 'wafer'):
            self.update_visualization(self.wafer)
