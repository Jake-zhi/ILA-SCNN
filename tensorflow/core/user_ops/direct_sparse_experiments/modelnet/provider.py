import os
import sys
import numpy as np
import h5py
import math
from operator import itemgetter

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(BASE_DIR)

# Download dataset for point cloud classification
DATA_DIR = os.path.join(BASE_DIR, 'data')
if not os.path.exists(DATA_DIR):
    os.mkdir(DATA_DIR)
if not os.path.exists(os.path.join(DATA_DIR, 'modelnet40_ply_hdf5_2048')):
    www = 'https://shapenet.cs.stanford.edu/media/modelnet40_ply_hdf5_2048.zip'
    zipfile = os.path.basename(www)
    os.system('wget %s; unzip %s' % (www, zipfile))
    os.system('mv %s %s' % (zipfile[:-4], DATA_DIR))
    os.system('rm %s' % (zipfile))


def shuffle_data(data, labels):
    """ Shuffle data and labels.
        Input:
          data: B,N,... numpy array
          label: B,... numpy array
        Return:
          shuffled data, label and shuffle indices
    """
    idx = np.arange(len(labels))
    np.random.shuffle(idx)
    return data[idx, ...], labels[idx], idx


def rotate_point_cloud(batch_data):
    """ Randomly rotate the point clouds to augument the dataset
        rotation is per shape based along up direction
        Input:
          BxNx3 array, original batch of point clouds
        Return:
          BxNx3 array, rotated batch of point clouds
    """
    rotated_data = np.zeros(batch_data.shape, dtype=np.float32)
    for k in range(batch_data.shape[0]):
        rotation_angle = np.random.uniform() * 2 * np.pi
        cosval = np.cos(rotation_angle)
        sinval = np.sin(rotation_angle)
        rotation_matrix = np.array([[cosval, 0, sinval],
                                    [0, 1, 0],
                                    [-sinval, 0, cosval]])
        shape_pc = batch_data[k, ...]
        rotated_data[k, ...] = np.dot(shape_pc.reshape((-1, 3)), rotation_matrix)
    return rotated_data


def rotate_point_cloud_by_angle(batch_data, rotation_angle):
    """ Rotate the point cloud along up direction with certain angle.
        Input:
          BxNx3 array, original batch of point clouds
        Return:
          BxNx3 array, rotated batch of point clouds
    """
    rotated_data = np.zeros(batch_data.shape, dtype=np.float32)
    for k in range(batch_data.shape[0]):
        #rotation_angle = np.random.uniform() * 2 * np.pi
        cosval = np.cos(rotation_angle)
        sinval = np.sin(rotation_angle)
        rotation_matrix = np.array([[cosval, 0, sinval],
                                    [0, 1, 0],
                                    [-sinval, 0, cosval]])
        shape_pc = batch_data[k, ...]
        rotated_data[k, ...] = np.dot(shape_pc.reshape((-1, 3)), rotation_matrix)
    return rotated_data


def jitter_point_cloud(batch_data, sigma=0.01, clip=0.05):
    """ Randomly jitter points. jittering is per point.
        Input:
          BxNx3 array, original batch of point clouds
        Return:
          BxNx3 array, jittered batch of point clouds
    """
    B, N, C = batch_data.shape
    assert(clip > 0)
    jittered_data = np.clip(sigma * np.random.randn(B, N, C), -1*clip, clip)
    jittered_data += batch_data
    return jittered_data

def getBoundingBox(points):
  min_x = 100000
  max_x = -100000
  min_y = 100000
  max_y = -100000
  min_z = 100000
  max_z = -100000
  max_x = points[:, :, 0].max()
  max_y = points[:, :, 1].max()
  max_z = points[:, :, 2].max()
  min_x = points[:, :, 0].min()
  min_y = points[:, :, 1].min()
  min_z = points[:, :, 2].min()
  return [min_x, min_y, min_z, max_x, max_y, max_z]

def toVoxelGrid(points, grid_size, res):
  [min_x, min_y, min_z, max_x, max_y, max_z] = getBoundingBox(points)
  max_size = max([max_x - min_x, max_y - min_y, max_z - min_z, grid_size])
  scale = float(max_size) / float(res) + 0.001
  sparse_grid = {}
  batch = 0
  for batch_data in points:
    for point in batch_data:
      bin_x = int(math.floor((point[0] - min_x) / scale))
      bin_y = int(math.floor((point[1] - min_y) / scale))
      bin_z = int(math.floor((point[2] - min_z) / scale))
      pid = (int(batch), bin_x, bin_y, bin_z, 0)
      sparse_grid[pid] = float(1)
    batch = batch + 1
  sorted_grid = sorted(sparse_grid.keys(), key=lambda x: x[0])
  sparse_points = np.array(sorted_grid, dtype=np.int64)
  sparse_values = np.array(sparse_grid.values(), dtype=np.float32)
  return [sparse_points, sparse_values]

def getDataFiles(list_filename):
    return [line.rstrip() for line in open(list_filename)]

def load_h5(h5_filename):
    f = h5py.File(h5_filename)
    data = f['data'][:]
    label = f['label'][:]
    return (data, label)

def loadDataFile(filename):
    return load_h5(filename)

def load_h5_data_label_seg(h5_filename):
    f = h5py.File(h5_filename)
    data = f['data'][:]
    label = f['label'][:]
    seg = f['pid'][:]
    return (data, label, seg)


def loadDataFile_with_seg(filename):
    return load_h5_data_label_seg(filename)
