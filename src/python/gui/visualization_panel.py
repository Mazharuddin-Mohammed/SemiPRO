from PySide6.QtWidgets import QWidget

class VisualizationPanel(QWidget):
    def __init__(self, width: int, height: int):
        super().__init__()
        # Import renderer from simulator to avoid direct cython dependency
        try:
            from ..simulator import PyVulkanRenderer
            self.renderer = PyVulkanRenderer(width, height)
            self.renderer.initialize()
        except ImportError:
            # Fallback to mock renderer
            self.renderer = None
        self.show_temperature_overlay = False
        self.show_reliability = False

    def update_visualization(self, wafer):
        if self.renderer:
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
