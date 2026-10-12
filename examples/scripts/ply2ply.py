import open3d as o3d

pcd = o3d.io.read_point_cloud('/home/flo/Documents/reset/envs/examples/scenes/tt_francis/point_cloud(old).ply')
print(pcd)
o3d.io.write_point_cloud('/home/flo/Documents/reset/envs/examples/scenes/tt_francis/point_cloud.ply', pcd, write_ascii=True)
