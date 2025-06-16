# Author: Dr. Mazharuddin Mohammed
"""
SemiPRO WebSocket API
====================

Real-time WebSocket API for SemiPRO simulation monitoring and control.
Provides live updates for simulation progress, status changes, and system events.
"""

import asyncio
import json
import time
import logging
from typing import Dict, Set, Any, Optional
from datetime import datetime
import weakref

try:
    import websockets
    from websockets.server import WebSocketServerProtocol
    WEBSOCKETS_AVAILABLE = True
except ImportError:
    WEBSOCKETS_AVAILABLE = False
    WebSocketServerProtocol = None

from .serializers import serialize_to_json

logger = logging.getLogger(__name__)

class WebSocketAPI:
    """WebSocket API for real-time communication"""
    
    def __init__(self, host: str = "localhost", port: int = 8765):
        self.host = host
        self.port = port
        self.server = None
        
        # Connection management
        self.connections: Set[WebSocketServerProtocol] = set()
        self.simulation_subscribers: Dict[str, Set[WebSocketServerProtocol]] = {}
        self.status_subscribers: Set[WebSocketServerProtocol] = set()
        
        # Message queues
        self.message_queue = asyncio.Queue()
        self.running = False
        
        # Statistics
        self.stats = {
            'connections_total': 0,
            'connections_active': 0,
            'messages_sent': 0,
            'messages_received': 0,
            'start_time': None
        }
    
    async def start_server(self):
        """Start the WebSocket server"""
        if not WEBSOCKETS_AVAILABLE:
            logger.error("WebSockets library not available. Install with: pip install websockets")
            return False
        
        try:
            self.server = await websockets.serve(
                self.handle_connection,
                self.host,
                self.port
            )
            
            self.running = True
            self.stats['start_time'] = time.time()
            
            # Start message processor
            asyncio.create_task(self.process_messages())
            
            logger.info(f"WebSocket server started on ws://{self.host}:{self.port}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to start WebSocket server: {e}")
            return False
    
    async def stop_server(self):
        """Stop the WebSocket server"""
        self.running = False
        
        if self.server:
            self.server.close()
            await self.server.wait_closed()
            logger.info("WebSocket server stopped")
        
        # Close all connections
        if self.connections:
            await asyncio.gather(
                *[conn.close() for conn in self.connections],
                return_exceptions=True
            )
            self.connections.clear()
    
    async def handle_connection(self, websocket: WebSocketServerProtocol, path: str):
        """Handle new WebSocket connection"""
        self.connections.add(websocket)
        self.stats['connections_total'] += 1
        self.stats['connections_active'] = len(self.connections)
        
        client_info = f"{websocket.remote_address[0]}:{websocket.remote_address[1]}"
        logger.info(f"New WebSocket connection from {client_info} on path {path}")
        
        try:
            # Send welcome message
            welcome_msg = {
                'type': 'welcome',
                'timestamp': datetime.utcnow().isoformat(),
                'server_info': {
                    'version': '1.0.0',
                    'capabilities': ['simulation_monitoring', 'status_updates', 'real_time_data']
                }
            }
            await self.send_message(websocket, welcome_msg)
            
            # Handle path-based subscriptions
            if path.startswith('/simulation/'):
                task_id = path.split('/')[-1]
                await self.subscribe_to_simulation(websocket, task_id)
            elif path == '/status':
                await self.subscribe_to_status(websocket)
            
            # Listen for messages
            async for message in websocket:
                await self.handle_message(websocket, message)
                
        except websockets.exceptions.ConnectionClosed:
            logger.info(f"WebSocket connection closed: {client_info}")
        except Exception as e:
            logger.error(f"WebSocket connection error: {e}")
        finally:
            # Cleanup
            await self.cleanup_connection(websocket)
    
    async def handle_message(self, websocket: WebSocketServerProtocol, message: str):
        """Handle incoming WebSocket message"""
        try:
            data = json.loads(message)
            self.stats['messages_received'] += 1
            
            msg_type = data.get('type')
            
            if msg_type == 'subscribe_simulation':
                task_id = data.get('task_id')
                if task_id:
                    await self.subscribe_to_simulation(websocket, task_id)
            
            elif msg_type == 'unsubscribe_simulation':
                task_id = data.get('task_id')
                if task_id:
                    await self.unsubscribe_from_simulation(websocket, task_id)
            
            elif msg_type == 'subscribe_status':
                await self.subscribe_to_status(websocket)
            
            elif msg_type == 'unsubscribe_status':
                await self.unsubscribe_from_status(websocket)
            
            elif msg_type == 'ping':
                await self.send_message(websocket, {'type': 'pong', 'timestamp': datetime.utcnow().isoformat()})
            
            elif msg_type == 'get_stats':
                await self.send_stats(websocket)
            
            else:
                await self.send_message(websocket, {
                    'type': 'error',
                    'message': f'Unknown message type: {msg_type}'
                })
                
        except json.JSONDecodeError:
            await self.send_message(websocket, {
                'type': 'error',
                'message': 'Invalid JSON message'
            })
        except Exception as e:
            logger.error(f"Error handling message: {e}")
            await self.send_message(websocket, {
                'type': 'error',
                'message': str(e)
            })
    
    async def subscribe_to_simulation(self, websocket: WebSocketServerProtocol, task_id: str):
        """Subscribe to simulation updates"""
        if task_id not in self.simulation_subscribers:
            self.simulation_subscribers[task_id] = set()
        
        self.simulation_subscribers[task_id].add(websocket)
        
        await self.send_message(websocket, {
            'type': 'subscription_confirmed',
            'subscription': 'simulation',
            'task_id': task_id
        })
        
        logger.info(f"WebSocket subscribed to simulation {task_id}")
    
    async def unsubscribe_from_simulation(self, websocket: WebSocketServerProtocol, task_id: str):
        """Unsubscribe from simulation updates"""
        if task_id in self.simulation_subscribers:
            self.simulation_subscribers[task_id].discard(websocket)
            
            if not self.simulation_subscribers[task_id]:
                del self.simulation_subscribers[task_id]
        
        await self.send_message(websocket, {
            'type': 'subscription_cancelled',
            'subscription': 'simulation',
            'task_id': task_id
        })
    
    async def subscribe_to_status(self, websocket: WebSocketServerProtocol):
        """Subscribe to general status updates"""
        self.status_subscribers.add(websocket)
        
        await self.send_message(websocket, {
            'type': 'subscription_confirmed',
            'subscription': 'status'
        })
        
        logger.info("WebSocket subscribed to status updates")
    
    async def unsubscribe_from_status(self, websocket: WebSocketServerProtocol):
        """Unsubscribe from status updates"""
        self.status_subscribers.discard(websocket)
        
        await self.send_message(websocket, {
            'type': 'subscription_cancelled',
            'subscription': 'status'
        })
    
    async def cleanup_connection(self, websocket: WebSocketServerProtocol):
        """Clean up connection and subscriptions"""
        self.connections.discard(websocket)
        self.status_subscribers.discard(websocket)
        
        # Remove from simulation subscriptions
        for task_id, subscribers in list(self.simulation_subscribers.items()):
            subscribers.discard(websocket)
            if not subscribers:
                del self.simulation_subscribers[task_id]
        
        self.stats['connections_active'] = len(self.connections)
    
    async def send_message(self, websocket: WebSocketServerProtocol, message: Dict[str, Any]):
        """Send message to specific WebSocket"""
        try:
            if websocket.open:
                await websocket.send(json.dumps(message))
                self.stats['messages_sent'] += 1
        except Exception as e:
            logger.error(f"Failed to send WebSocket message: {e}")
    
    async def broadcast_to_simulation_subscribers(self, task_id: str, message: Dict[str, Any]):
        """Broadcast message to simulation subscribers"""
        if task_id in self.simulation_subscribers:
            subscribers = self.simulation_subscribers[task_id].copy()
            
            if subscribers:
                await asyncio.gather(
                    *[self.send_message(ws, message) for ws in subscribers],
                    return_exceptions=True
                )
    
    async def broadcast_to_status_subscribers(self, message: Dict[str, Any]):
        """Broadcast message to status subscribers"""
        if self.status_subscribers:
            subscribers = self.status_subscribers.copy()
            await asyncio.gather(
                *[self.send_message(ws, message) for ws in subscribers],
                return_exceptions=True
            )
    
    async def broadcast_to_all(self, message: Dict[str, Any]):
        """Broadcast message to all connected clients"""
        if self.connections:
            connections = self.connections.copy()
            await asyncio.gather(
                *[self.send_message(ws, message) for ws in connections],
                return_exceptions=True
            )
    
    async def send_simulation_progress(self, task_id: str, progress_data: Dict[str, Any]):
        """Send simulation progress update"""
        message = {
            'type': 'simulation_progress',
            'task_id': task_id,
            'timestamp': datetime.utcnow().isoformat(),
            'data': progress_data
        }
        
        await self.broadcast_to_simulation_subscribers(task_id, message)
    
    async def send_simulation_status(self, task_id: str, status: str, details: Optional[Dict[str, Any]] = None):
        """Send simulation status update"""
        message = {
            'type': 'simulation_status',
            'task_id': task_id,
            'status': status,
            'timestamp': datetime.utcnow().isoformat()
        }
        
        if details:
            message['details'] = details
        
        await self.broadcast_to_simulation_subscribers(task_id, message)
    
    async def send_system_status(self, status_data: Dict[str, Any]):
        """Send system status update"""
        message = {
            'type': 'system_status',
            'timestamp': datetime.utcnow().isoformat(),
            'data': status_data
        }
        
        await self.broadcast_to_status_subscribers(message)
    
    async def send_stats(self, websocket: WebSocketServerProtocol):
        """Send server statistics"""
        current_time = time.time()
        uptime = current_time - self.stats['start_time'] if self.stats['start_time'] else 0
        
        stats = {
            'type': 'server_stats',
            'timestamp': datetime.utcnow().isoformat(),
            'stats': {
                **self.stats,
                'uptime_seconds': uptime,
                'simulation_subscriptions': len(self.simulation_subscribers),
                'status_subscriptions': len(self.status_subscribers)
            }
        }
        
        await self.send_message(websocket, stats)
    
    async def process_messages(self):
        """Process queued messages"""
        while self.running:
            try:
                # Process any queued messages
                while not self.message_queue.empty():
                    message = await self.message_queue.get()
                    await self.handle_queued_message(message)
                
                await asyncio.sleep(0.1)  # Small delay to prevent busy waiting
                
            except Exception as e:
                logger.error(f"Error processing messages: {e}")
    
    async def handle_queued_message(self, message: Dict[str, Any]):
        """Handle queued message"""
        msg_type = message.get('type')
        
        if msg_type == 'simulation_progress':
            await self.send_simulation_progress(
                message['task_id'],
                message['data']
            )
        elif msg_type == 'simulation_status':
            await self.send_simulation_status(
                message['task_id'],
                message['status'],
                message.get('details')
            )
        elif msg_type == 'system_status':
            await self.send_system_status(message['data'])
        elif msg_type == 'broadcast':
            await self.broadcast_to_all(message['data'])
    
    def queue_simulation_progress(self, task_id: str, progress_data: Dict[str, Any]):
        """Queue simulation progress update"""
        message = {
            'type': 'simulation_progress',
            'task_id': task_id,
            'data': progress_data
        }
        
        try:
            self.message_queue.put_nowait(message)
        except asyncio.QueueFull:
            logger.warning("WebSocket message queue is full")
    
    def queue_simulation_status(self, task_id: str, status: str, details: Optional[Dict[str, Any]] = None):
        """Queue simulation status update"""
        message = {
            'type': 'simulation_status',
            'task_id': task_id,
            'status': status
        }
        
        if details:
            message['details'] = details
        
        try:
            self.message_queue.put_nowait(message)
        except asyncio.QueueFull:
            logger.warning("WebSocket message queue is full")
    
    def queue_system_status(self, status_data: Dict[str, Any]):
        """Queue system status update"""
        message = {
            'type': 'system_status',
            'data': status_data
        }
        
        try:
            self.message_queue.put_nowait(message)
        except asyncio.QueueFull:
            logger.warning("WebSocket message queue is full")

# Global WebSocket API instance
_websocket_api = None

def get_websocket_api() -> Optional[WebSocketAPI]:
    """Get global WebSocket API instance"""
    return _websocket_api

def initialize_websocket_api(host: str = "localhost", port: int = 8765) -> WebSocketAPI:
    """Initialize global WebSocket API instance"""
    global _websocket_api
    _websocket_api = WebSocketAPI(host, port)
    return _websocket_api

async def start_websocket_server(host: str = "localhost", port: int = 8765) -> bool:
    """Start WebSocket server"""
    api = initialize_websocket_api(host, port)
    return await api.start_server()

async def stop_websocket_server():
    """Stop WebSocket server"""
    if _websocket_api:
        await _websocket_api.stop_server()

# Convenience functions for sending updates
def notify_simulation_progress(task_id: str, progress_data: Dict[str, Any]):
    """Notify simulation progress (non-blocking)"""
    if _websocket_api:
        _websocket_api.queue_simulation_progress(task_id, progress_data)

def notify_simulation_status(task_id: str, status: str, details: Optional[Dict[str, Any]] = None):
    """Notify simulation status change (non-blocking)"""
    if _websocket_api:
        _websocket_api.queue_simulation_status(task_id, status, details)

def notify_system_status(status_data: Dict[str, Any]):
    """Notify system status (non-blocking)"""
    if _websocket_api:
        _websocket_api.queue_system_status(status_data)
